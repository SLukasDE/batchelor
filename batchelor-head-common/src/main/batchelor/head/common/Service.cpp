/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#include <batchelor/common/CRC32.h>
#include <batchelor/common/Timestamp.h>
#include <batchelor/common/types/State.h>

#include <batchelor/head/common/Service.h>
#include <batchelor/head/common/Logger.h>

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>

namespace batchelor {
namespace head {
namespace common {
namespace {
Logger logger("batchelor::head::common::Service");


service::schemas::RunResponse makeRunResponse(const Dao::Task& task) {
	service::schemas::RunResponse rv;

	rv.taskId = task.taskId;
	rv.message = batchelor::common::types::State::toString(task.state);

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
	return batchelor::common::CRC32().pushData(crc32Str.c_str(), crc32Str.size()).get();
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
	rv.state = batchelor::common::types::State::toString(task.state);
	rv.returnCode = task.returnCode;
	rv.message = task.message;
	rv.tsCreated = batchelor::common::Timestamp::toJSON(task.createdTS);
	rv.tsRunning = batchelor::common::Timestamp::toJSON(task.startTS);
	rv.tsFinished = batchelor::common::Timestamp::toJSON(task.endTS);
	rv.tsLastHeartBeat = batchelor::common::Timestamp::toJSON(task.lastHeartbeatTS);

	return rv;
}

}

Service::Service(const esl::object::Context& aContext, RequestHandler& aRequestHandler)
: context(aContext),
  requestHandler(aRequestHandler)
{ }

service::schemas::FetchResponse Service::fetchTask(const service::schemas::FetchRequest& fetchRequest) {
	logger.trace << "fetchTask\n";
	service::schemas::FetchResponse rv;

	for(const auto& taskStatus : fetchRequest.tasks) {
		std::unique_ptr<Dao::Task> existingTask = getDao().loadTaskByTaskId(taskStatus.taskId);
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

		getDao().updateTask(*existingTask);
		requestHandler.onUpdateTask(*existingTask);
	}

	std::vector<Dao::Task> tasks;
	for(const auto eventType : fetchRequest.eventTypes) {
		if(eventType.available) {
			std::vector<Dao::Task> eventTypeTasks = getDao().loadTasksByEventTypeAndState(eventType.eventType, batchelor::common::types::State::queued);
			tasks.insert(tasks.end(), eventTypeTasks.begin(), eventTypeTasks.end());
		}
	}
	std::sort(std::begin(tasks), std::end(tasks), [](const Dao::Task &a, const Dao::Task &b) {
		return a.effectivePriority > b.effectivePriority || (a.effectivePriority == b.effectivePriority && a.createdTS < b.createdTS);
	});

	std::vector<service::schemas::Setting> metrics = getMetrics(fetchRequest.metrics);
	for(auto& task : tasks) {
		if(evaluateCondition(metrics, task.condition)) {
			task.state = batchelor::common::types::State::running;
			task.startTS = task.lastHeartbeatTS = std::chrono::system_clock::now();
			task.metrics = metrics;
			getDao().updateTask(task);
			requestHandler.onUpdateTask(task);

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

std::vector<service::schemas::TaskStatusHead> Service::getTasks(const std::string& state, const std::string& eventNotAfterStr, const std::string& eventNotBeforeStr) {
	logger.trace << "getTasks\n";
	std::vector<service::schemas::TaskStatusHead> rv;

	std::chrono::system_clock::time_point eventNotAfter;
	if(!eventNotAfterStr.empty()) {
		eventNotAfter = batchelor::common::Timestamp::fromJSON(eventNotAfterStr);
	}

	std::chrono::system_clock::time_point eventNotBefore;
	if(!eventNotBeforeStr.empty()) {
		eventNotBefore = batchelor::common::Timestamp::fromJSON(eventNotBeforeStr);
	}

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
	if(existingTask && (existingTask->state == batchelor::common::types::State::queued || existingTask->state == batchelor::common::types::State::running)) {
		existingTask->priority = runRequest.priority;
		existingTask->settings = runRequest.settings;
		existingTask->condition = runRequest.condition;
		getDao().updateTask(*existingTask);
		requestHandler.onUpdateTask(*existingTask);

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
		task.state = batchelor::common::types::State::Type::queued;

		getDao().saveTask(task);
		requestHandler.onUpdateTask(task);

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
	requestHandler.onUpdateTask(*task);
}

esl::database::Connection& Service::getDBConnection() const {
	if(!dbConnection) {
		dbConnection = requestHandler.getDbConnectionFactory().createConnection();
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

} /* namespace common */
} /* namespace head */
} /* namespace batchelor */
