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

#include <batchelor/worker/AliveRequestHandler.h>
#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/Procedure.h>
#include <batchelor/worker/TaskFailed.h>

#include <batchelor/common/Timestamp.h>
#include <batchelor/common/types/State.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/TaskStatusWorker.h>

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/client/exception/NetworkError.h>
#include <esl/com/http/server/MHDSocket.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <algorithm>
#include <set>
#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace worker {

namespace {
Logger logger("batchelor::worker::Procedure");
boost::uuids::random_generator rg;
AliveRequestHandler aliveRequestHandler;

class ScopeGuard {
public:
	ScopeGuard(std::function<void()> aLambda)
	: lambda(aLambda),
	  empty(false)
	{ }

	ScopeGuard(const ScopeGuard &) = delete;

	ScopeGuard(ScopeGuard && other)
	: lambda(std::move(other.lambda)),
	  empty(false)
	{
		other.empty = true;
	}

	~ScopeGuard() {
		if(!empty) {
			lambda();
		}
	}

	ScopeGuard & operator=(const ScopeGuard &) = delete;

	ScopeGuard & operator=(ScopeGuard && other) {
		lambda = std::move(other.lambda);
		empty = false;
		other.empty = true;
		return *this;
	}

	void clear() {
		empty = true;
	}

private:
	std::function<void()> lambda;
    bool empty = true;
};

std::map<std::string, int> substractResources(std::map<std::string, int> resourcesAvailable, const std::map<std::string, int>& resourcesAllocated) {
	for(const auto& resourceAllocated : resourcesAllocated) {
		auto availableIter = resourcesAvailable.insert(std::make_pair(resourceAllocated.first, 0));
		availableIter.first->second = std::max<int>(0, availableIter.first->second - resourceAllocated.second);
	}

	return resourcesAvailable;
}
}

Procedure::Settings::Settings() {
	boost::uuids::uuid taskIdUUID = rg();
	workerId = boost::uuids::to_string(taskIdUUID);
}

Procedure::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
    for(const auto& setting : settings) {
		/*
		if(setting.first.size() > 18 && setting.first.substr(0, 18) == "resource-available") {
			std::string key = setting.first.substr(18);
			float value = 0;

			try {
				value = std::stof(setting.second);
			}
			catch(const std::invalid_argument& e) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			catch(const std::out_of_range& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" for parameter \"" + setting.first + "\" is out of range.");
			}
			if(value <= 0) {
				throw std::runtime_error("Value for parameter \"" + setting.first + "\" must be greater than 0 but it is \"" + setting.second + "\"");
			}

			if(resourcesAvailable.insert(std::make_pair(key, value)).second == false) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}
		}
		*/
		if(setting.first.size() > 7 && setting.first.substr(0, 7) == "metric.") {
			std::string key = setting.first.substr(7);
			metrics.emplace_back(key, setting.second);
		}

		else if(setting.first == "maximum-tasks-running") {
			if(maximumTasksRunning != std::string::npos) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
			}

			try {
				maximumTasksRunning = esl::utility::String::toNumber<std::size_t>(setting.second);
			}
			catch(const std::exception& e) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'." + e.what()));
			}
		}

		else if(setting.first == "idle-timeout") {
			if(idleTimeout.count() > 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
			}

			try {
				idleTimeout = common::Timestamp::toDuration(setting.second);
			}
			catch(const std::exception& e) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'." + e.what()));
			}
		}

		else if(setting.first == "available-timeout") {
			if(availableTimeout.count() > 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
			}

			try {
				availableTimeout = common::Timestamp::toDuration(setting.second);
			}
			catch(const std::exception& e) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'." + e.what()));
			}
		}

		else if(setting.first == "task-factory-id") {
			if(taskFactoryIds.insert(setting.second).second == false) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute \"" + setting.first + "\"='" + setting.second + "'."));
			}
		}

		else if(setting.first == "batchelor-head-server-id") {
			if(connectionFactoryIds.insert(setting.second).second == false) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute \"" + setting.first + "\"='" + setting.second + "'."));
			}
		}

		else if(setting.first == "worker-id") {
			if(!workerId.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute \"" + setting.first + "\"='" + setting.second + "'."));
			}
			workerId = setting.second;
			if(workerId.empty()) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'."));
			}
		}

		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Key \"" + setting.first + "\" is unknown"));
		}
    }

	if(workerId.empty()) {
		// TODO: Check if there is an environment variable set to specify the worker-id
		boost::uuids::uuid taskIdUUID = rg();
		workerId = boost::uuids::to_string(taskIdUUID);
	}
}

Procedure::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings) {
	for(const auto& taskFactoryId : settings.taskFactoryIds) {
		plugin::TaskFactory& taskFactory = context.getObject<plugin::TaskFactory>(taskFactoryId);
		if(taskFactroyByEventType.emplace(taskFactoryId, std::ref(taskFactory)).second == false) {
			throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of task factory id \"" + taskFactoryId + "\"."));
		}

		const std::map<std::string, int>& resourcesRequired = taskFactory.getResourcesRequired();
		for(const auto& resourceRequired : resourcesRequired) {
			if(resourcesAvailable.find(resourceRequired.first) != resourcesAvailable.end()) {
				continue;
			}

			bool metricFound = false;
			for(const auto metric : settings.metrics) {
				if(metric.first == resourceRequired.first) {
					metricFound = true;

					int value = 0;
					try {
						value = std::stoi(metric.second);
					}
					catch(const std::invalid_argument& e) {
			            throw esl::system::Stacktrace::add(std::runtime_error("Task factory \"" + taskFactoryId + "\" requires resource '" + resourceRequired.first + "' with invalid value \"" + metric.second + "\". " + e.what()));
					}
					catch(const std::out_of_range& e) {
			            throw esl::system::Stacktrace::add(std::runtime_error("Task factory \"" + taskFactoryId + "\" requires resource '" + resourceRequired.first + "' with a value \"" + metric.second + "\" that is out of range. " + e.what()));
					}
					if(value == 0) {
			            throw esl::system::Stacktrace::add(std::runtime_error("Task factory \"" + taskFactoryId + "\" requires resource '" + resourceRequired.first + "' with a value \"" + metric.second + "\" smaller than 0."));
					}
					resourcesAvailable[resourceRequired.first] = value;

					break;
				}
			}
			if(metricFound == false) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Task factory \"" + taskFactoryId + "\" requires a non existing resource '" + resourceRequired.first + "'."));
			}
		}
	}

	for(const auto& connectionId : settings.connectionFactoryIds) {
		common::plugin::ConnectionFactory& connectionFactory = context.getObject<common::plugin::ConnectionFactory>(connectionId);
		connectionFactories.emplace_back(connectionId, std::ref(connectionFactory));
	}
}

std::unique_ptr<esl::object::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Procedure>(new Procedure(Procedure::Settings(settings)));
}

Procedure::Procedure(const Settings& aSettings)
: settings(aSettings)
{
	if(settings.connectionFactoryIds.empty()) {
		throw std::runtime_error("No connections defined");
	}
	if(settings.alivePort > 0) {
		std::vector<std::pair<std::string, std::string>> mhdSettings;
		mhdSettings.emplace_back("port", std::to_string(settings.alivePort));
		mhdSettings.emplace_back("https", "false");
		socket = esl::com::http::server::MHDSocket::createNative(esl::com::http::server::MHDSocket::Settings(mhdSettings));
	}
}

Procedure::~Procedure() {
	procedureCancel();
	socketMutex.lock();
}

void Procedure::internalProcedureRun(esl::object::Context& context) {
    idleTimeAt = std::chrono::steady_clock::now() + settings.idleTimeout;
    unavailableTimeAt = std::chrono::steady_clock::now() + settings.availableTimeout;

	logger.debug << "Idle timeout:" << settings.idleTimeout.count() << "\n";
	logger.debug << "Available timeout:" << settings.availableTimeout.count() << "\n";

	if(socket) {
		socketMutex.lock();
		ScopeGuard scopeGuard([&]() {
			socketMutex.unlock();
		});
		socket->listen(aliveRequestHandler, [&]{
			socketMutex.unlock();
		});
		scopeGuard.clear();
	}

    /* ************* *
	 *      run      *
	 * ************* */
	ScopeGuard scopeGuard([&]() {
		if(socket) {
			socket->release();
		}
	});

	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	bool doWait = false;
	while(true) {
		if(doWait) {
			notifyCV.wait_for(lockNotifyMutex, settings.requestInterval);
		}
		if(signalsReceived > signalsProcessed) {
			++signalsProcessed;
			if(signalsReceivedMax == std::string::npos || signalsReceivedMax > signalsReceived) {
				signalTasks("interrupt");
				signalTasks("terminate");
				signalTasks("pipe");
			}
			else {
				signalTasks("kill");
			}
		}
		doWait = !runResilient();

		if(settings.idleTimeout.count() > 0 && std::chrono::steady_clock::now() > idleTimeAt) {
			logger.info << "Idle timeout occurred.\n";
			return;
		}

		if(availableTimeoutOccurred) {
			logger.info << "Available timeout occurred.\n";
			if(taskByTaskId.empty()) {
				return;
			}
		}

		//std::cerr << "someone set stopThread to " << stopThread << ", taskByTaskId.size() == " << taskByTaskId.size() << "\n";
		if(signalsReceived > 0 && taskByTaskId.empty()) {
			return;
		}
	}
}

void Procedure::procedureCancel() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		++signalsReceived;
	}
	notifyCV.notify_one();
}

void Procedure::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

std::vector<std::pair<std::string, std::string>> Procedure::getCurrentMetrics() const {
	return settings.metrics;
}

std::vector<std::pair<std::string, std::string>> Procedure::getCurrentMetrics(const service::schemas::RunConfiguration& runConfiguration) const {
	std::vector<std::pair<std::string, std::string>> rv = getCurrentMetrics();

	rv.push_back(std::make_pair("TASK_ID", runConfiguration.taskId));
	rv.push_back(std::make_pair("EVENT_TYPE", runConfiguration.eventType));

	return rv;
}

void Procedure::signalTasks(const std::string& signal) {
	for(const auto& task : taskByTaskId) {
		if(task.second->getStatus().state == common::types::State::running) {
			task.second->sendSignal(signal);
		}
	}
}

bool Procedure::runResilient() {
	std::size_t firstConnectionFactory = nextConnectionFactory;
	do {
		try {
			return run();
		}
		catch(const esl::com::http::client::exception::NetworkError& e) {
	        std::cerr << "NetworkError occurred: " << e.what() << "\n";
			httpConnectionFactory = nullptr;
		}
		/* It is intended to stop the application, if an exception occurs.
		 * (maybe there is a DB error, serialization error or other std::runtime_error)
		 * But if there is an Network-Error, then we want to retry with another connection factory */
#if 0
	    catch(const std::exception& e) {
	        std::cerr << "Exception occurred: " << e.what() << "\n";
			httpConnectionFactory.reset();
	    }
	    catch(...) {
	        std::cerr << "Unknown exception occurred.\n";
			httpConnectionFactory.reset();
	    }
#endif
	} while(firstConnectionFactory != nextConnectionFactory);

    logger.debug << "Sleep...\n";

	return false;
}

bool Procedure::run() {
	bool actionReceived = false;
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	/********************************************************
	 * prepare data to send on performing request fetchTask *
	 ********************************************************/

	service::schemas::FetchRequest fetchRequest;

	/* prepare task status list and count running threads */

	/* calculate resources */
	std::size_t tasksRunning = 0;
	std::map<std::string, int> resourcesAllocated;
	std::vector<std::string> notRunningTaskIDs;
	for(const auto& task : taskByTaskId) {
		service::schemas::TaskStatusWorker taskStatusWorker;

		taskStatusWorker.taskId = task.first;
		auto taskStatus = task.second->getStatus();
		taskStatusWorker.state = common::types::State::toString(taskStatus.state);
		taskStatusWorker.returnCode = taskStatus.returnCode;
		taskStatusWorker.message = taskStatus.message;

		fetchRequest.tasks.push_back(taskStatusWorker);

		if(taskStatus.state == common::types::State::running) {
			++tasksRunning;
			for(const auto& resourceAllocated : taskStatus.resourcesAllocated) {
				resourcesAllocated[resourceAllocated.first] += resourceAllocated.second;
			}
		}
		else {
			notRunningTaskIDs.push_back(task.first);
		}
	}
	std::map<std::string, int> resourcesAvailable = substractResources(initializedSettings->resourcesAvailable, resourcesAllocated);

	fetchRequest.workerId = settings.workerId;

	/* prepare list of metrics for transmission */

	std::vector<std::pair<std::string, std::string>> metrics = getCurrentMetrics();
	for(const auto& metric : metrics) {
		auto iter = resourcesAvailable.find(metric.first);
		if(iter == resourcesAvailable.end()) {
			fetchRequest.metrics.push_back(service::schemas::Setting::make(metric.first, metric.second));
		}
		else {
			fetchRequest.metrics.push_back(service::schemas::Setting::make(metric.first, std::to_string(iter->second)));
		}
	}
	fetchRequest.metrics.push_back(service::schemas::Setting::make("TASKS_RUNNING", std::to_string(tasksRunning)));


	/* prepare the list of available event types and if they are available to create a new task */

	if(settings.availableTimeout.count() > 0 && std::chrono::steady_clock::now() > unavailableTimeAt) {
		availableTimeoutOccurred = true;
	}
	else if(signalsReceived == 0 && initializedSettings) {
		for(const auto& taskFactory : initializedSettings->taskFactroyByEventType) {
			service::schemas::EventTypeAvailable eventTypeAvailable;

			eventTypeAvailable.eventType = taskFactory.first;
			eventTypeAvailable.available = (settings.maximumTasksRunning == 0 || settings.maximumTasksRunning > tasksRunning) && !taskFactory.second.get().isBusy(resourcesAvailable);

			fetchRequest.eventTypes.push_back(eventTypeAvailable);
		}
	}

	/*********************************
	 * Perform the fetchTask request *
	 *********************************/
	service::schemas::FetchResponse fetchResponse = client.fetchTask(settings.namespaceId, fetchRequest);


	/* send signals to tasks */

	for(const auto& signal : fetchResponse.signals) {
		auto iter = taskByTaskId.find(signal.taskId);
		if(iter == taskByTaskId.end()) {
			logger.warn << "Received a message to send a signal to an unknown task \"" << signal.taskId << "\"\n";
		}
		else if(iter->second->getStatus().state == common::types::State::running) {
			iter->second->sendSignal(signal.signal);
			actionReceived = true;
		}
	}

	for(const auto& notRunningTaskID: notRunningTaskIDs) {
		taskByTaskId.erase(notRunningTaskID);
	}

	for(const auto& runConfiguration : fetchResponse.runConfigurations) {
		if(!initializedSettings) {
			logger.warn << "Received a message to create a tasks for event type \"" << runConfiguration.eventType << "\", but inizializeContext has not been called.\n";
			continue;
		}
		auto iter = initializedSettings->taskFactroyByEventType.find(runConfiguration.eventType);
		if(iter == initializedSettings->taskFactroyByEventType.end()) {
			logger.warn << "Received a message to create a tasks for an unknown event type \"" << runConfiguration.eventType << "\"\n";
			continue;
		}

		if(taskByTaskId.count(runConfiguration.taskId)) {
			logger.warn << "Received a message to create a tasks for event type \"" << runConfiguration.eventType << "\" and task id \"" << runConfiguration.taskId << "\", but there exists already a task with same task id.\n";
			continue;
		}

		std::unique_ptr<plugin::Task> task;
		try {
			task = iter->second.get().createTask(notifyCV, notifyMutex, getCurrentMetrics(runConfiguration), runConfiguration);
		}
		catch(const std::exception& e) {
			logger.warn << "Could not create task " << runConfiguration.taskId << " for event type \"" << runConfiguration.eventType << "\" because exception occured with message \"" << e.what() << "\".\n";
			plugin::Task::Status taskStatus;

			taskStatus.state = common::types::State::Type::signaled;
			taskStatus.returnCode = -1;
			taskStatus.message = e.what();

			task.reset(new TaskFailed(std::move(taskStatus)));
		}
		catch(...) { }

		if(!task) {
			logger.warn << "Could not create task " << runConfiguration.taskId << " for event type \"" << runConfiguration.eventType << "\".\n";
			plugin::Task::Status taskStatus;

			taskStatus.state = common::types::State::Type::signaled;
			taskStatus.returnCode = -1;
			taskStatus.message = "creating task failed";

			task.reset(new TaskFailed(std::move(taskStatus)));
		}

		taskByTaskId.insert(std::make_pair(runConfiguration.taskId, std::move(task)));
		actionReceived = true;
	}

	if(tasksRunning > 0 || actionReceived) {
	    idleTimeAt = std::chrono::steady_clock::now() + settings.idleTimeout;
	}

	return actionReceived;
}

std::unique_ptr<esl::com::http::client::Connection> Procedure::createHTTPConnection() const {
	if(!httpConnectionFactory && initializedSettings && nextConnectionFactory < initializedSettings->connectionFactories.size()) {
		httpConnectionFactory = &initializedSettings->connectionFactories[nextConnectionFactory].second.get();
		nextConnectionFactory = (nextConnectionFactory + 1) % initializedSettings->connectionFactories.size();
	}
	if(!httpConnectionFactory) {
		if(!initializedSettings) {
			logger.warn << "InizializeContext has not been called.\n";
		}
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}

	auto httpConnection = httpConnectionFactory->get().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

} /* namespace worker */
} /* namespace batchelor */
