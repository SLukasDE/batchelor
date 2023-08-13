#include <batchelor/head/CRC32.h>
#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Signal.h>

#include <batchelor/common/types/State.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <time.h>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
#include <iomanip>
#include <sstream>
#endif

#include <iostream>
namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Service");

// creates a string in ISO 8601 format, e.g. '2012-04-21T18:25:43-05:00'
std::string toJSONTimestamp(const std::chrono::time_point<std::chrono::system_clock>& time_point) {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
	std::ostringstream ss;
	std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
	std::tm tm = *std::localtime(&tt);
	ss << std::put_time(&tm, "%FT%T%z");
	return ss.str();
#else
	auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch());
	std::time_t timestamp = millisecs.count() / 1000;
	int millisec = static_cast<int>(millisecs.count() % 1000);

	std::tm tm;
	localtime_r(&timestamp, &tm);

	char buffer[100];
	char* bufferPtr = &buffer[0];
	std::size_t length;

	length = strftime(bufferPtr, 100, "%FT%T", &tm);
	if(length == 0) {
		return "";
	}
	bufferPtr += length;

	sprintf(bufferPtr, ".%03d", millisec);
	bufferPtr += 4;

	length = strftime(bufferPtr, 100-length-4, "%z", &tm);
	if(length == 0) {
		return "";
	}

	bufferPtr[length+1] = 0;
	bufferPtr[length-0] = bufferPtr[length-1];
	bufferPtr[length-1] = bufferPtr[length-2];
	bufferPtr[length-2] = ':';

	return buffer;
#endif
}

std::chrono::time_point<std::chrono::system_clock> fromJSONTimestamp(const std::string& str) {
	std::tm timeinfo;
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
	std::istringstream is{str};
	is.imbue(std::locale("de_DE.utf-8"));
	is >> std::get_time(&timeinfo, "%FT%T%z");
    if (is.fail()) {
        throw std::runtime_error("Parse failed");
    }
#else
    strptime(str.c_str(), "%FT%T%z", &timeinfo);
#endif
    std::time_t tt = std::mktime(&timeinfo);
    return std::chrono::system_clock::from_time_t(tt);
}

service::schemas::RunResponse makeRunResponse(const Dao::Task& task) {
	service::schemas::RunResponse rv;

	rv.taskId = task.taskId;
	rv.message = common::types::State::toString(task.state);

	return rv;
}

std::uint32_t makeCrc32(const service::schemas::RunRequest& runRequest) {
	std::string crc32Str;
	for(const auto& setting : runRequest.settings) {
		if(!crc32Str.empty()) {
			crc32Str += ";";
		}
		crc32Str += setting.key + "=" + setting.value;
	}
	return CRC32().pushData(crc32Str.c_str(), crc32Str.size()).get();
}

std::vector<service::schemas::Setting> getMetrics(const std::vector<service::schemas::Setting>& metrics) {
	return metrics;
}

bool evaluateCondition(const std::vector<service::schemas::Setting>& metrics, const std::string& condition) {
	return true;
}

service::schemas::TaskStatusHead taskToTaskStatusHead(const Dao::Task& task) {
	service::schemas::TaskStatusHead rv;

	rv.runConfiguration.eventType = task.eventType;
	rv.runConfiguration.taskId = task.taskId;
	rv.runConfiguration.settings = task.settings;
	/* Metrics contains all metric variables and their values as used for the condition at the time the task has been assigned to a worker and state changed to running.
	 * Available variables to get used in the formula are all variables delivered as metrics of "fetch-request" like
	 * - cpu usage               (CPU_USAGE),
	 * - memory usage            (MEM_USAGE),
	 * - number of running tasks (TASKS_RUNNING),
	 * - host name               (HOST_NAME),
	 * - cloudId,
	 * - workerId
	 * - ...
	 * as well as task specific variables provided by the head server, like
	 * - waiting time,
	 * - priority,
	 * - ...
	 */
	rv.metrics = task.metrics;
	rv.state = common::types::State::toString(task.state);
	rv.returnCode = task.returnCode;
	rv.message = task.message;
	rv.tsCreated = toJSONTimestamp(task.createdTS);
	rv.tsRunning = toJSONTimestamp(task.startTS);
	rv.tsFinished = toJSONTimestamp(task.endTS);
	rv.tsLastHeartBeat = toJSONTimestamp(task.lastHeartbeatTS);

	return rv;
}

}

Service::Service(const esl::object::Context& aContext, esl::database::ConnectionFactory& aDbConnectionFactory)
: context(aContext),
  dbConnectionFactory(aDbConnectionFactory)
{ }

service::schemas::FetchResponse Service::fetchTask(const service::schemas::FetchRequest& fetchRequest) {
	logger.trace << "fetchTask\n";
    // siegburger str schleumer
	// 30.8., 31.8, 1.9.
	service::schemas::FetchResponse rv;

	for(const auto& taskStatus : fetchRequest.tasks) {
		std::unique_ptr<Dao::Task> existingTask = getDao().loadTaskByTaskId(taskStatus.taskId);
		if(!existingTask) {
			logger.warn << "Worker sent an update for a non existing task \"" << taskStatus.taskId << "\"\n.";
			continue;
		}
		if(existingTask->state != common::types::State::running) {
			logger.warn << "Worker sent an update for a non running task \"" << taskStatus.taskId << "\"\n.";
			continue;
		}

		existingTask->state = common::types::State::toState(taskStatus.state);
		existingTask->returnCode = taskStatus.returnCode;
		existingTask->message = taskStatus.message;
		existingTask->lastHeartbeatTS = std::chrono::system_clock::now();

		if(existingTask->state == common::types::State::running) {
			for(const auto& signalId : existingTask->signals) {
				service::schemas::Signal signal;
				signal.signal = signalId;
				signal.taskId = existingTask->taskId;
				rv.signals.push_back(signal);
			}
			existingTask->signals.clear();
		}
		else {
			existingTask->endTS = existingTask->lastHeartbeatTS;
		}

		getDao().updateTask(*existingTask);
	}

	std::vector<Dao::Task> tasks;
	for(const auto eventType : fetchRequest.eventTypes) {
		if(eventType.available) {
			std::vector<Dao::Task> eventTypeTasks = getDao().loadTasksByEventTypeAndState(eventType.eventType, common::types::State::queued);
			tasks.insert(tasks.end(), eventTypeTasks.begin(), eventTypeTasks.end());
		}
	}
	std::sort(std::begin(tasks), std::end(tasks), [](const Dao::Task &a, const Dao::Task &b) {
		return a.effectivePriority < b.effectivePriority || (a.effectivePriority == b.effectivePriority && a.createdTS > b.createdTS);
	});

	std::vector<service::schemas::Setting> metrics = getMetrics(fetchRequest.metrics);
	for(auto& task : tasks) {
		if(evaluateCondition(metrics, task.condition)) {
			task.state = common::types::State::running;
			task.startTS = task.lastHeartbeatTS = std::chrono::system_clock::now();
			task.metrics = metrics;
			getDao().updateTask(task);

			service::schemas::RunConfiguration runConfiguration;

			runConfiguration.taskId = task.taskId;
			runConfiguration.eventType = task.eventType;
			runConfiguration.settings = task.settings;

			rv.runConfigurations.push_back(runConfiguration);

			break;
		}
	}

	return rv;
}

std::vector<service::schemas::TaskStatusHead> Service::getTasks(const std::string& stateStr, const std::string& eventNotAfterStr, const std::string& eventNotBeforeStr) {
	logger.trace << "getTasks\n";
	std::vector<service::schemas::TaskStatusHead> rv;

	auto state = common::types::State::toState(stateStr);
	auto eventNotAfter = fromJSONTimestamp(eventNotAfterStr);
	auto eventNotBefore = fromJSONTimestamp(eventNotBeforeStr);

	std::vector<Dao::Task> tasks = getDao().loadTasks(state, eventNotAfter, eventNotBefore);
	for(const auto& task : tasks) {
		rv.push_back(taskToTaskStatusHead(task));
	}

	return rv;
}


std::unique_ptr<service::schemas::TaskStatusHead> Service::getTask(const std::string& taskId) {
	logger.trace << "getTask\n";
	std::unique_ptr<service::schemas::TaskStatusHead> rv;
	std::unique_ptr<Dao::Task> task = getDao().loadTaskByTaskId(taskId);

	if(task) {
		rv.reset(new service::schemas::TaskStatusHead);
		*rv = taskToTaskStatusHead(*task);
	}

	return rv;
}

service::schemas::RunResponse Service::runTask(const service::schemas::RunRequest& runRequest) {
	logger.trace << "runTask\n";
	service::schemas::RunResponse rv;

	std::uint32_t crc32 = makeCrc32(runRequest);

	std::unique_ptr<Dao::Task> existingTask = getDao().loadLatesTaskByEventTypeAndCrc32(runRequest.eventType, crc32);
	if(existingTask && (existingTask->state == common::types::State::queued || existingTask->state == common::types::State::running)) {
		existingTask->priority = runRequest.priority;
		existingTask->settings = runRequest.settings;
		existingTask->condition = runRequest.condition;
		getDao().updateTask(*existingTask);

		rv = makeRunResponse(*existingTask);
	}
	else {
		//boost::uuids::uuid taskIdUUID; // initialize uuid
		static boost::uuids::random_generator rg;
		boost::uuids::uuid taskIdUUID = rg();

		Dao::Task task;
		task.taskId = boost::uuids::to_string(taskIdUUID);
		task.crc32 = crc32;
		task.eventType = runRequest.eventType;
		task.priority = runRequest.priority;
		task.settings = runRequest.settings;
#if 1
		task.createdTS = std::chrono::system_clock::now();
#else
		task.createdTS = toJSONTimestamp(std::chrono::system_clock::now());
#endif
		task.state = common::types::State::Type::queued;
		getDao().saveTask(task);

		rv = makeRunResponse(task);
	}

	//logger.info << "task.createdTS: " << toJSONTimestamp(task.createdTS) << "\n";

	//rv.taskId = "1234-ab3c-1234-ab3c-1234-ab3c-1234-ab3c";

	return rv;
}

void Service::sendSignal(const std::string& taskId, const std::string& signal) {
	logger.trace << "sendSignal\n";
	std::unique_ptr<Dao::Task> task = getDao().loadTaskByTaskId(taskId);
	if(!task) {
		throw esl::com::http::server::exception::StatusCode(404, esl::utility::MIME::Type::applicationJson, "{}");
	}

	task->signals.push_back(signal);
	getDao().updateTask(*task);
}

esl::database::Connection& Service::getDBConnection() const {
	if(!dbConnection) {
		dbConnection = dbConnectionFactory.createConnection();
	}

	if(!dbConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("no db connection available."));
	}

	return *dbConnection;
}

Dao& Service::getDao() const {
	if(!dao) {
		dao.reset(new Dao(getDBConnection()));
	}

	return *dao;
}


} /* namespace head */
} /* namespace batchelor */
