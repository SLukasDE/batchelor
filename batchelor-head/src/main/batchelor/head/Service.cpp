#include <batchelor/head/CRC32.h>
#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <batchelor/common/types/State.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <time.h>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Service");

std::string getJSONTimestamp(const std::chrono::time_point<std::chrono::system_clock>& time_point) {
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
}

service::schemas::RunResponse makeRunResponse(const Dao::Task& task) {
	service::schemas::RunResponse rv;

	rv.jobId = task.taskId;
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

}

Service::Service(const esl::object::Context& aContext, esl::database::ConnectionFactory& aDbConnectionFactory)
: context(aContext),
  dbConnectionFactory(aDbConnectionFactory)
{ }

service::schemas::FetchResponse Service::fetchTask(const service::schemas::FetchRequest& fetchRequest) {
	service::schemas::FetchResponse rv;

	return rv;
}

std::vector<service::schemas::JobStatusHead> Service::getTasks(const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) {
	std::vector<service::schemas::JobStatusHead> rv;

	return rv;
}

std::unique_ptr<service::schemas::JobStatusHead> Service::getTask(const std::string& taskId) {
	std::unique_ptr<service::schemas::JobStatusHead> rv;
	std::unique_ptr<Dao::Task> task = getDao().loadTaskByTaskId(taskId);
	if(task) {
		rv.reset(new service::schemas::JobStatusHead);

		rv->runConfiguration.eventType = task->eventType;
		rv->runConfiguration.jobId = task->taskId;
		rv->runConfiguration.settings = task->settings;

		/* Metrics contains all metric variables and their values as used for the condition at the time the job has been assigned to a worker and state changed to running.
		 * Available variables to get used in the formula are all variables delivered as metrics of "fetch-request" like
		 * - cpu usage              (CPU_USAGE),
		 * - memory usage           (MEM_USAGE),
		 * - number of jobs running (RUNNING_JOBS),
		 * - host name              (HOST_NAME),
		 * - cloudId,
		 * - workerId
		 * - ...
		 * as well as job specific variables provided by the head server, like
		 * - waiting time,
		 * - priority,
		 * - ...
		 */
		//rv->metrics;

		rv->state = common::types::State::toString(task->state);

		rv->returnCode = task->returnCode;
		rv->message = task->message;

		rv->tsCreated = getJSONTimestamp(task->createdTS);
		rv->tsRunning = getJSONTimestamp(task->startTS);
		rv->tsFinished = getJSONTimestamp(task->endTS);
		rv->tsLastHeartBeat = getJSONTimestamp(task->lastHeartbeatTS);
	}
	return rv;
}

service::schemas::RunResponse Service::runTask(const service::schemas::RunRequest& runRequest) {
	service::schemas::RunResponse rv;

	std::uint32_t crc32 = makeCrc32(runRequest);

	std::unique_ptr<Dao::Task> existingTask = getDao().loadLatesQueuedOrRunningTaskByCrc32(runRequest.eventType, crc32);
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
		task.createdTS = getJSONTimestamp(std::chrono::system_clock::now());
#endif
		task.state = common::types::State::Type::queued;
		getDao().saveTask(task);

		rv = makeRunResponse(task);
	}

	//logger.info << "job.createdTS: " << getJSONTimestamp(job.createdTS) << "\n";

	//rv.jobId = "1234-ab3c-1234-ab3c-1234-ab3c-1234-ab3c";

	return rv;
}

void Service::sendSignal(const std::string& taskId, const std::string& signal) {
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
