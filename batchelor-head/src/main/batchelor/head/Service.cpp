/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <batchelor/common/auth/UserData.h>
#include <batchelor/common/Timestamp.h>
#include <batchelor/common/types/State.h>

#include <batchelor/condition/Compiler.h>
#include <batchelor/condition/Scanner.h>

#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/object/Value.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/CRC32.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Service");


service::schemas::RunResponse makeRunResponse(const Dao::Task& task) {
	service::schemas::RunResponse rv;

	rv.taskId = task.taskId;
	rv.message = batchelor::common::types::State::toString(task.state);

	return rv;
}

service::schemas::RunResponse makeRunResponse(const std::string& errorMessage) {
	service::schemas::RunResponse rv;

	rv.taskId = "";
	rv.message = errorMessage;

	return rv;
}

std::uint32_t makeCrc32(const service::schemas::RunRequest& runRequest) {
	std::string crc32Str;

	// Condition and priority may be changed.
	// crc32Str = runRequest.condition;

	for(const auto& setting : runRequest.settings) {
		crc32Str += ";" + setting.key + "=" + setting.value;
	}
	for(const auto& metric : runRequest.metrics) {
		crc32Str += ";" + metric.key + "=" + metric.value;
	}

	return esl::utility::CRC32().pushData(crc32Str.c_str(), crc32Str.size()).get();
}

bool evaluateCondition(const std::vector<service::schemas::Setting>& metrics, const std::string& condition) {
	if(condition.empty()) {
		return true;
	}

	condition::Compiler compiler;
	bool rv = false;

	for(const auto& metric : metrics) {
		compiler.addVariable(metric.key, metric.value);
	}

#if 0
std::cout << "METRICS:" << std::endl;
for(const auto& metric : metrics) {
std::cout << "- \"" << metric.key << "\" = \"" << metric.value << "\"" << std::endl;
}
std::cout << "CONDITION: '" << condition << "'" << std::endl;
#endif

	std::stringstream str;
	str << condition;
	condition::Scanner scanner(str);

	try {
		compiler.parse(scanner);
	}
	catch(const std::exception& e) {
		logger.error << "Exception occurred while parsing condition \"" << condition << "\": \"" << e.what() << "\n";
		return false;
	}
	catch(...) {
		logger.error << "Exception occurred while parsing condition \"" << condition << "\".\n";
		return false;
	}

	try {
		rv = compiler.toBool();
	}
	catch(const std::exception& e) {
		logger.error << "Exception occurred while executing condition \"" << condition << "\": \"" << e.what() << "\n";
		return false;
	}
	catch(...) {
		logger.error << "Exception occurred while executing condition \"" << condition << "\".\n";
		return false;
	}

	return rv;
}

service::schemas::TaskStatusHead taskToTaskStatusHead(const Dao::Task& task) {
	service::schemas::TaskStatusHead rv;

	rv.runConfiguration.eventType = task.eventType;
	rv.runConfiguration.taskId = task.taskId;
	rv.runConfiguration.settings = task.settings;
	rv.runConfiguration.metrics = task.metrics;
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
//	rv.metrics = task.metrics;
	rv.state = batchelor::common::types::State::toString(task.state);
	rv.condition = task.condition;
	rv.returnCode = task.returnCode;
	rv.message = task.message;
	rv.tsCreated = batchelor::common::Timestamp::toJSON(task.createdTS);
	rv.tsRunning = batchelor::common::Timestamp::toJSON(task.startTS);
	rv.tsFinished = batchelor::common::Timestamp::toJSON(task.endTS);
	rv.tsLastHeartBeat = batchelor::common::Timestamp::toJSON(task.lastHeartbeatTS);

	return rv;
}

void addOrReplaceMetric(std::vector<service::schemas::Setting>& metrics, const std::string& key, const std::string& value) {
	for(auto& metric : metrics) {
		if(metric.key == key) {
			metric.value = value;
			return;
		}
	}
	metrics.emplace_back(service::schemas::Setting::make(key, value));
}

}

Service::Service(const esl::object::Context& aContext, Engine& aEngine, std::mutex& mutex)
: context(aContext),
  engine(aEngine),
  lockMutex(mutex)
{ }

void Service::alive() {
	logger.trace << "Service call: \"alive\"\n";
}

service::schemas::FetchResponse Service::fetchTask(const std::string& namespaceId, const service::schemas::FetchRequest& fetchRequest) {
	logger.trace << "Service call: \"fetchTask\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::worker) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	service::schemas::FetchResponse rv;

	for(const auto& taskStatus : fetchRequest.tasks) {
		std::unique_ptr<Dao::Task> existingTask = getDao().loadTaskByTaskId(namespaceId, taskStatus.taskId);
		if(!existingTask) {
			logger.warn << "Worker sent an update for a non existing task \"" << taskStatus.taskId << "\"\n.";
			continue;
		}
		if(existingTask->state != batchelor::common::types::State::running) {
			logger.warn << "Worker sent an update for a non running task \"" << taskStatus.taskId << "\"\n.";
			continue;
		}

		existingTask->state = batchelor::common::types::State::toState(taskStatus.state);
		existingTask->returnCode = taskStatus.returnCode;
		existingTask->message = taskStatus.message;
		existingTask->lastHeartbeatTS = std::chrono::system_clock::now();

		if(existingTask->state == batchelor::common::types::State::running) {
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

		getDao().updateTask(namespaceId, *existingTask);
		engine.onUpdateTask(*existingTask);
	}

	std::vector<Dao::Task> tasks;
	std::vector<std::pair<std::string, std::string>> eventTypes;
	for(const auto eventType : fetchRequest.eventTypes) {
		if(eventType.available) {
			std::vector<Dao::Task> eventTypeTasks = getDao().loadTasksByEventTypeAndState(namespaceId, eventType.eventType, batchelor::common::types::State::queued);
			tasks.insert(tasks.end(), eventTypeTasks.begin(), eventTypeTasks.end());
		}
		eventTypes.emplace_back(namespaceId, eventType.eventType);
	}
	std::sort(std::begin(tasks), std::end(tasks), [](const Dao::Task& a, const Dao::Task& b) {
		return a.effectivePriority > b.effectivePriority || (a.effectivePriority == b.effectivePriority && a.createdTS < b.createdTS);
	});
	getDao().updateEventTypes(eventTypes);

	for(auto& task : tasks) {
		// Initialize 'metrics' with metrics set by batchelor control
		std::vector<service::schemas::Setting> metrics = task.metrics;

		// add or replace metrics with metrics provided by the worker
		for(const auto& fetchRequestMetric : fetchRequest.metrics) {
			addOrReplaceMetric(metrics, fetchRequestMetric.key, fetchRequestMetric.value);
		}




		// add or replace metrics with metrics calculated by head-server, e.g. waiting time
		std::chrono::system_clock::time_point nowTS = std::chrono::system_clock::now();

		auto secondsWaiting = std::chrono::duration_cast<std::chrono::seconds>(nowTS-task.createdTS).count();
		addOrReplaceMetric(metrics, "SECONDS_WAITING", std::to_string(secondsWaiting));

		auto minutesWaiting = std::chrono::duration_cast<std::chrono::minutes>(nowTS-task.createdTS).count();
		addOrReplaceMetric(metrics, "MINUTES_WAITING", std::to_string(minutesWaiting));




		if(!evaluateCondition(metrics, task.condition)) {
			continue;
		}

		task.state = batchelor::common::types::State::running;
		task.returnCode = 0;
		task.startTS = task.lastHeartbeatTS = std::chrono::system_clock::now();
		task.metrics = metrics;

		getDao().updateTask(namespaceId, task);
		engine.onUpdateTask(task);

		service::schemas::RunConfiguration runConfiguration;

		runConfiguration.taskId = task.taskId;
		runConfiguration.eventType = task.eventType;
		runConfiguration.settings = task.settings;
		runConfiguration.metrics = task.metrics;
		rv.runConfigurations.push_back(runConfiguration);

		break;
	}

	return rv;
}

std::vector<service::schemas::TaskStatusHead> Service::getTasks(const std::string& namespaceId, const std::string& state, const std::string& eventNotAfterStr, const std::string& eventNotBeforeStr) {
	logger.trace << "Service call: \"getTasks\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::readOnly) == 0 && roles.count(common::auth::UserData::Role::execute) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	std::vector<service::schemas::TaskStatusHead> rv;

	std::chrono::system_clock::time_point eventNotAfter;
	if(!eventNotAfterStr.empty()) {
		eventNotAfter = batchelor::common::Timestamp::fromJSON(eventNotAfterStr);
	}

	std::chrono::system_clock::time_point eventNotBefore;
	if(!eventNotBeforeStr.empty()) {
		eventNotBefore = batchelor::common::Timestamp::fromJSON(eventNotBeforeStr);
	}

	std::vector<Dao::Task> tasks = getDao().loadTasks(namespaceId, state, eventNotAfter, eventNotBefore);
	for(const auto& task : tasks) {
		rv.push_back(taskToTaskStatusHead(task));
	}

	return rv;
}


std::unique_ptr<service::schemas::TaskStatusHead> Service::getTask(const std::string& namespaceId, const std::string& taskId) {
	logger.trace << "Service call: \"getTask\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::readOnly) == 0 && roles.count(common::auth::UserData::Role::execute) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	std::unique_ptr<service::schemas::TaskStatusHead> rv;
	std::unique_ptr<Dao::Task> task = getDao().loadTaskByTaskId(namespaceId, taskId);

	if(task) {
		rv.reset(new service::schemas::TaskStatusHead);
		*rv = taskToTaskStatusHead(*task);
	}

	return rv;
}

service::schemas::RunResponse Service::runTask(const std::string& namespaceId, const service::schemas::RunRequest& runRequest) {
	logger.trace << "Service call: \"runTask\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::execute) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	if(!runRequest.condition.empty()) {
		condition::Compiler compiler;

		std::stringstream str;
		str << runRequest.condition;
		condition::Scanner scanner(str);

		try {
			compiler.parse(scanner);
		}
		catch(const std::exception& e) {
			return makeRunResponse("Exception occurred while parsing condition \"" + runRequest.condition + "\": \"" + e.what());
		}
		catch(...) {
			return makeRunResponse("Exception occurred while parsing condition \"" + runRequest.condition + "\".");
		}
	}

	service::schemas::RunResponse rv;

	// calculates CRC32 from  and runRequest.metrics
	std::uint32_t crc32 = makeCrc32(runRequest);

	std::unique_ptr<Dao::Task> existingTask = getDao().loadLatesTaskByEventTypeAndCrc32(namespaceId, runRequest.eventType, crc32);
	if(existingTask && (existingTask->state == batchelor::common::types::State::queued || existingTask->state == batchelor::common::types::State::running)) {
		existingTask->priority = runRequest.priority;
		//existingTask->settings = runRequest.settings;
		//existingTask->metrics = runRequest.metrics;
		existingTask->condition = runRequest.condition;
		getDao().updateTask(namespaceId, *existingTask);
		engine.onUpdateTask(*existingTask);

		rv = makeRunResponse(*existingTask);
	}
	else {
		{
			bool eventTypeAvailable = false;
			auto availableEventTypes = getDao().loadEventTypes(namespaceId);
			for(const auto& availableEventType : availableEventTypes) {
				if(availableEventType == runRequest.eventType) {
					eventTypeAvailable = true;
					break;
				}
			}

			if(!eventTypeAvailable) {
				return makeRunResponse("Event type is not available");
				//throw esl::com::http::server::exception::StatusCode(404, esl::utility::MIME::Type::applicationJson, "message: Event type '" + runRequest.eventType + "' is not available");
			}
		}

		//boost::uuids::uuid taskIdUUID; // initialize uuid
		static boost::uuids::random_generator rg;
		boost::uuids::uuid taskIdUUID = rg();

		Dao::Task task;
		task.taskId = boost::uuids::to_string(taskIdUUID);
		task.crc32 = crc32;
		task.eventType = runRequest.eventType;
		task.priority = runRequest.priority;
		task.settings = runRequest.settings;
		task.metrics = runRequest.metrics;
		task.condition = runRequest.condition;
#if 1
		task.createdTS = std::chrono::system_clock::now();
#else
		task.createdTS = toJSONTimestamp(std::chrono::system_clock::now());
#endif
		task.state = batchelor::common::types::State::Type::queued;

		getDao().saveTask(namespaceId, task);
		engine.onUpdateTask(task);

		rv = makeRunResponse(task);
	}

	return rv;
}

void Service::sendSignal(const std::string& namespaceId, const std::string& taskId, const std::string& signal) {
	logger.trace << "Service call: \"sendSignal\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::execute) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	std::unique_ptr<Dao::Task> task = getDao().loadTaskByTaskId(namespaceId, taskId);
	if(!task) {
		throw esl::com::http::server::exception::StatusCode(404, esl::utility::MIME::Type::applicationJson, "{}");
	}

	if(task->state == common::types::State::queued || task->state == common::types::State::running) {

		if(task->state == common::types::State::queued) {
			task->state = common::types::State::signaled;
		}

		if(task->state == common::types::State::running) {
			task->signals.push_back(signal);
		}

		getDao().updateTask(namespaceId, *task);
		engine.onUpdateTask(*task);
	}
}

std::vector<std::string> Service::getEventTypes(const std::string& namespaceId) {
	logger.trace << "Service call: \"getEventTypes\"\n";

	auto roles = common::auth::UserData::getRoles(context, namespaceId);
	if(roles.count(common::auth::UserData::Role::readOnly) == 0 && roles.count(common::auth::UserData::Role::execute) == 0) {
		throw esl::com::http::server::exception::StatusCode(401);
	}

	return getDao().loadEventTypes(namespaceId);
}

esl::database::Connection& Service::getDBConnection() const {
	if(!dbConnection) {
		dbConnection = engine.getDbConnectionFactory().createConnection();
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
