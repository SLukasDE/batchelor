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

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/Main.h>
#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/TaskFailed.h>

#include <batchelor/common/types/State.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/TaskStatusWorker.h>

#include <zsystem4esl/system/signal/Signal.h>

#include <esl/com/http/client/exception/NetworkError.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/Signal.h>
#include <esl/utility/String.h>

#include <set>
#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace worker {

namespace {
Logger logger("batchelor::worker::Main");
}

Main::Main(const Settings& aSettings)
: settings(aSettings),
  signal(new zsystem4esl::system::signal::Signal({}))
{
	if(settings.servers.empty()) {
		throw std::runtime_error("No servers defined");
	}

	for(const auto& event : settings.events) {
		std::unique_ptr<plugin::TaskFactory> taskFactory = esl::plugin::Registry::get().create<plugin::TaskFactory>(event.type, event.settings);
		if(taskFactroyByEventType.insert(std::make_pair(event.id, std::move(taskFactory))).second == false) {
			throw std::runtime_error("Cannot add an event type with id \"" + event.id + "\" because there exists already an event type with same id.");
		}
	}


	/* ********************** *
	 * use own signal handler *
	 * ********************** */

	std::set<std::string> stopSignals { {"interrupt"}, {"terminate"}, {"pipe"}};
	for(auto stopSignal : stopSignals) {
		signalHandles.push_back(signal->createHandler(esl::utility::Signal(stopSignal), [this]() {
			stopRunning();
		}));
	}


	/* ************* *
	 *      run      *
	 * ************* */
	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	bool doWait = false;
	while(true) {
		if(doWait) {
			notifyCV.wait_for(lockNotifyMutex, std::chrono::milliseconds(5000));
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

		//std::cerr << "someone set stopThread to " << stopThread << ", taskByTaskId.size() == " << taskByTaskId.size() << "\n";
		if(signalsReceived > 0 && taskByTaskId.empty()) {
			return;
		}
	}
}

Main::~Main() {
	stopRunning();
}

int Main::getReturnCode() const noexcept {
	return rc;
}

std::vector<std::pair<std::string, std::string>> Main::getCurrentMetrics() const {
	return settings.metrics;
}

std::vector<std::pair<std::string, std::string>> Main::getCurrentMetrics(const service::schemas::RunConfiguration& runConfiguration) const {
	std::vector<std::pair<std::string, std::string>> rv = getCurrentMetrics();

	rv.push_back(std::make_pair("TASK_ID", runConfiguration.taskId));
	rv.push_back(std::make_pair("EVENT_TYPE", runConfiguration.eventType));

	return rv;
}

void Main::stopRunning() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		++signalsReceived;
	}
	notifyCV.notify_one();
}

void Main::signalTasks(const std::string& signal) {
	for(const auto& task : taskByTaskId) {
		if(task.second->getStatus().state == common::types::State::running) {
			task.second->sendSignal(signal);
		}
	}
}

bool Main::runResilient() {
	std::size_t firstServer = nextServer;
	do {
		try {
			return run();
		}
#if 1
		catch(const esl::com::http::client::exception::NetworkError& e) {
	        std::cerr << "NetworkError occurred: " << e.what() << "\n";
			httpConnectionFactory.reset();
		}
#else
	    catch(const std::exception& e) {
	        std::cerr << "Exception occurred: " << e.what() << "\n";
	    }
	    catch(...) {
	        std::cerr << "Unknown exception occurred.\n";
	    }
		httpConnectionFactory.reset();
#endif
	} while(firstServer != nextServer);

    std::cerr << "Sleep...\n";
	return false;
}

bool Main::run() {
	bool actionReceived = false;
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	/********************************************************
	 * prepare data to send on performing request fetchTask *
	 ********************************************************/

	service::schemas::FetchRequest fetchRequest;


	/* prepare task status list and count running threads */

	std::size_t tasksRunning = 0;
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
		}
		else {
std::cout << "Task " << task.first << " finished with state \"" << taskStatusWorker.state << "\"\n";
			notRunningTaskIDs.push_back(task.first);
		}
	}


	/* prepare list of metrics for transmission */

	std::vector<std::pair<std::string, std::string>> metrics = getCurrentMetrics();
	for(const auto& metric : metrics) {
		fetchRequest.metrics.push_back(service::schemas::Setting::make(metric.first, metric.second));
	}
	fetchRequest.metrics.push_back(service::schemas::Setting::make("TASKS_RUNNING", std::to_string(tasksRunning)));


	/* prepare the list of available event types and if they are available to create a new task */

	if(signalsReceived == 0) {
		for(const auto& taskFactory : taskFactroyByEventType) {
			service::schemas::EventTypeAvailable eventTypeAvailable;

			eventTypeAvailable.eventType = taskFactory.first;
			eventTypeAvailable.available = (settings.maximumTasksRunning == 0 || settings.maximumTasksRunning > tasksRunning) && !taskFactory.second->isBusy();

			fetchRequest.eventTypes.push_back(eventTypeAvailable);
		}
	}

	/*********************************
	 * Perform the fetchTask request *
	 *********************************/
	service::schemas::FetchResponse fetchResponse = client.fetchTask(fetchRequest);


	/* send signals to tasks */

	for(const auto& signal : fetchResponse.signals) {
		auto iter = taskByTaskId.find(signal.taskId);
		if(iter == taskByTaskId.end()) {
			logger.warn << "Received a message to send a signal to an unknown task \"" << signal.taskId << "\"\n";
		}
		else if(iter->second->getStatus().state == common::types::State::running) {
std::cout << "Signal \"" << signal.signal << "\" received for task " << signal.taskId << ".\n";
			iter->second->sendSignal(signal.signal);
			actionReceived = true;
		}
	}

	for(const auto& notRunningTaskID: notRunningTaskIDs) {
		taskByTaskId.erase(notRunningTaskID);
	}

	for(const auto& runConfiguration : fetchResponse.runConfigurations) {
		auto iter = taskFactroyByEventType.find(runConfiguration.eventType);
		if(iter == taskFactroyByEventType.end()) {
			logger.warn << "Received a message to create a tasks for an unknown event type \"" << runConfiguration.eventType << "\"\n";
			continue;
		}

		if(taskByTaskId.count(runConfiguration.taskId)) {
			logger.warn << "Received a message to create a tasks for event type \"" << runConfiguration.eventType << "\" and task id \"" << runConfiguration.taskId << "\", but there exists already a task with same task id.\n";
			continue;
		}



		std::unique_ptr<plugin::Task> task;
		try {
			task = iter->second->createTask(notifyCV, notifyMutex, getCurrentMetrics(runConfiguration), runConfiguration);
		}
		catch(const std::exception& e) {
std::cerr << "Could not create task " << runConfiguration.taskId << " for event type \"" << runConfiguration.eventType << "\" because exception occured with message \"" << e.what() << "\".\n";
			plugin::Task::Status taskStatus;

			taskStatus.state = common::types::State::Type::signaled;
			taskStatus.returnCode = -1;
			taskStatus.message = e.what();

			task.reset(new TaskFailed(std::move(taskStatus)));
		}
		catch(...) { }

		if(!task) {
std::cerr << "Could not create task " << runConfiguration.taskId << " for event type \"" << runConfiguration.eventType << "\".\n";
			plugin::Task::Status taskStatus;

			taskStatus.state = common::types::State::Type::signaled;
			taskStatus.returnCode = -1;
			taskStatus.message = "creating task failed";

			task.reset(new TaskFailed(std::move(taskStatus)));
		}
else {
std::cout << "Task " << runConfiguration.taskId << " created for event type \"" << runConfiguration.eventType << "\".\n";
}

		taskByTaskId.insert(std::make_pair(runConfiguration.taskId, std::move(task)));
		actionReceived = true;
	}

	return actionReceived;
}

std::unique_ptr<esl::com::http::client::Connection> Main::createHTTPConnection() {
	auto httpConnection = getHTTPConnectionFactory().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

esl::com::http::client::ConnectionFactory& Main::getHTTPConnectionFactory() {
	if(!httpConnectionFactory) {
		std::string eslPlugin = settings.servers[nextServer].plugin;
		eslPlugin = "eslx/com/http/client/ConnectionFactory";

		std::vector<std::pair<std::string, std::string>> eslSettings;
		eslSettings = settings.servers[nextServer].settings;
		eslSettings.push_back(std::make_pair("url", settings.servers[nextServer].url));
		if(!settings.servers[nextServer].username.empty()) {
			eslSettings.push_back(std::make_pair("username", settings.servers[nextServer].username));
		}
		if(!settings.servers[nextServer].password.empty()) {
			eslSettings.push_back(std::make_pair("password", settings.servers[nextServer].password));
		}

		httpConnectionFactory = esl::plugin::Registry::get().create<esl::com::http::client::ConnectionFactory>(eslPlugin, eslSettings);
		nextServer = (nextServer + 1) % settings.servers.size();
	}
	if(!httpConnectionFactory) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}
	return *httpConnectionFactory;
}

} /* namespace worker */
} /* namespace batchelor */
