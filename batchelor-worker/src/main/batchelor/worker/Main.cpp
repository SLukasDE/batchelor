#include <batchelor/worker/config/Config.h>
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

Main::Main(const Options& options)
: signal(new zsystem4esl::system::signal::Signal({}))
{
	url = "http://127.0.0.1:8080";

	userDefinedMetrics = options.getMetrics();
	setMaximumTasksRunning(options.getMaximumTasksRunning());

	for(const auto& configFile : options.getConfigFiles()) {
		config::Config(*this, configFile);
	}

	std::set<std::string> stopSignals { {"interrupt"}, {"terminate"}, {"pipe"}};
	for(auto stopSignal : stopSignals) {
		signalHandles.push_back(signal->createHandler(esl::utility::Signal(stopSignal), [this]() {
			stopRunning();
		}));
	}

	run1();
}

void Main::stopRunning() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		++signalsReceived;
	}
	notifyCV.notify_one();
}

int Main::getReturnCode() const noexcept {
	return rc;
}

void Main::setMaximumTasksRunning(std::size_t aMaximumTasksRunning) {
	maximumTasksRunning = aMaximumTasksRunning;
}

std::size_t Main::getMaximumTasksRunning() const noexcept {
	return maximumTasksRunning;
}

void Main::addEventType(const std::string& id, std::unique_ptr<plugin::TaskFactory> eventType) {
	if(taskFactroyByEventType.insert(std::make_pair(id, std::move(eventType))).second == false) {
		throw std::runtime_error("Cannot add an event type with id \"" + id + "\" because there exists already an event type with same id.");
	}
}

void Main::addUserDefinedMetric(const std::string& key, const std::string& value) {
	userDefinedMetrics.emplace_back(key, value);
}

std::vector<std::pair<std::string, std::string>> Main::getCurrentMetrics() const {
	return userDefinedMetrics;
}

std::vector<std::pair<std::string, std::string>> Main::getCurrentMetrics(const service::schemas::RunConfiguration& runConfiguration) const {
	std::vector<std::pair<std::string, std::string>> rv = getCurrentMetrics();

	rv.push_back(std::make_pair("TASK_ID", runConfiguration.taskId));
	rv.push_back(std::make_pair("EVENT_TYPE", runConfiguration.eventType));

	return rv;
}

void Main::signalTasks(const std::string& signal) {
	for(const auto& task : taskByTaskId) {
		if(task.second->getStatus().state == common::types::State::running) {
			task.second->sendSignal(signal);
		}
	}
}

void Main::run1() {
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
		try {
			doWait = !run2();
		}
		catch(const esl::com::http::client::exception::NetworkError& e) {
			logger.warn << "function threw exception \"" << e.what() << "\"\n";
			throw;
		}
		catch(const std::exception& e) {
			logger.warn << "run2 function threw exception \"" << e.what() << "\"\n";
			doWait = true;
		}
		catch(...) {
			logger.warn << "run2 function threw exception\n";
			doWait = true;
		}

		//std::cerr << "someone set stopThread to " << stopThread << ", taskByTaskId.size() == " << taskByTaskId.size() << "\n";
		if(signalsReceived > 0 && taskByTaskId.empty()) {
			return;
		}
	}
}

bool Main::run2() {
	bool actionReceived = false;
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	service::schemas::FetchRequest fetchRequest;

	/* prepare transmission of task status list and count running threads */
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

	/* prepare transmission of metrics */
	std::vector<std::pair<std::string, std::string>> metrics = getCurrentMetrics();

	for(const auto& metric : metrics) {
		fetchRequest.metrics.push_back(service::schemas::Setting::make(metric.first, metric.second));
	}
	fetchRequest.metrics.push_back(service::schemas::Setting::make("TASKS_RUNNING", std::to_string(tasksRunning)));

	/* prepare transmission the list of available event types and if they are available to create a new task */
	if(signalsReceived == 0) {
		for(const auto& taskFactory : taskFactroyByEventType) {
			service::schemas::EventTypeAvailable eventTypeAvailable;

			eventTypeAvailable.eventType = taskFactory.first;
			eventTypeAvailable.available = (getMaximumTasksRunning() == 0 || getMaximumTasksRunning() > tasksRunning) && !taskFactory.second->isBusy();

			fetchRequest.eventTypes.push_back(eventTypeAvailable);
		}
	}

	service::schemas::FetchResponse fetchResponse = client.fetchTask(fetchRequest);

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
		}
		else if(taskByTaskId.count(runConfiguration.taskId)) {
			logger.warn << "Received a message to create a tasks for event type \"" << runConfiguration.eventType << "\" and task id \"" << runConfiguration.taskId << "\", but there exists already a task with same task id.\n";
		}
		else {
			std::vector<std::pair<std::string, std::string>> settings;
			for(const auto& setting : runConfiguration.settings) {
				settings.push_back(std::make_pair(setting.key, setting.value));
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
		httpConnectionFactory = esl::plugin::Registry::get().create<esl::com::http::client::ConnectionFactory>(
				"eslx/com/http/client/ConnectionFactory", {
						{"url", url}
				});
	}
	if(!httpConnectionFactory) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}
	return *httpConnectionFactory;
}

} /* namespace worker */
} /* namespace batchelor */
