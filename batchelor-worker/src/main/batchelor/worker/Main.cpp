#include <batchelor/worker/Logger.h>
#include <batchelor/worker/Main.h>
#include <batchelor/worker/TaskFailed.h>
#include <batchelor/worker/TaskStatus.h>

#include <batchelor/common/types/State.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/TaskStatusWorker.h>

#include <esl/com/http/client/exception/NetworkError.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/Signal.h>
#include <esl/utility/String.h>

#include <stdexcept>

#include <batchelor/worker/TaskFactoryExec.h>
namespace batchelor {
namespace worker {

namespace {
Logger logger("batchelor::worker::Main");
}

Main::Main(const Options& aOptions)
: options(aOptions)
{
	url = "http://127.0.0.1:8080";
	taskFactroyByEventType["wurst"] = TaskFactoryExec::create({ {{"cmd"},{"/bin/sleep"}} , {{"cd"},{"/tmp/wurst"}} });

	run1();
}

Main::~Main() {
}

void Main::stopRunning() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

		stopThread = true;
		signalTasks(esl::utility::Signal("interrupt"));
		signalTasks(esl::utility::Signal("terminate"));
		signalTasks(esl::utility::Signal("pipe"));
		auto waitUntilTimeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
		if(notifyCV.wait_until(lockNotifyMutex, waitUntilTimeout, [&]{
			for(const auto& task : taskByTaskId) {
				if(task.second->getTaskStatus().state == common::types::State::running) {
					return false;
				}
			}
			return true;
		}) == false) {
			signalTasks(esl::utility::Signal::Type::kill);
		}
	}
	notifyCV.notify_one();
}

int Main::getReturnCode() const {
	return rc;
}

void Main::signalTasks(const esl::utility::Signal& signal) {
	for(const auto& task : taskByTaskId) {
		if(task.second->getTaskStatus().state == common::types::State::running) {
			task.second->sendSignal(esl::utility::Signal(signal));
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
		if(stopThread) {
			return;
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
		auto taskStatus = task.second->getTaskStatus();
		taskStatusWorker.state = common::types::State::toString(taskStatus.state);
		taskStatusWorker.returnCode = taskStatus.returnCode;
		taskStatusWorker.message = taskStatus.message;

		fetchRequest.tasks.push_back(taskStatusWorker);

		if(taskStatus.state == common::types::State::running) {
			++tasksRunning;
		}
		else {
			notRunningTaskIDs.push_back(task.first);
		}
	}

	/* prepare transmission of metrics */
	for(const auto& metric : options.getMetrics()) {
		fetchRequest.metrics.push_back(service::schemas::Setting::make(metric.first, metric.second));
	}
	fetchRequest.metrics.push_back(service::schemas::Setting::make("TASKS_RUNNING", std::to_string(tasksRunning)));

	/* prepare transmission the list of available event types and if they are available to create a new task */
	for(const auto& taskFactory : taskFactroyByEventType) {
		service::schemas::EventTypeAvailable eventTypeAvailable;

		eventTypeAvailable.eventType = taskFactory.first;
		eventTypeAvailable.available = (options.getMaximumTasksRunning() == 0 || options.getMaximumTasksRunning() > tasksRunning) && !taskFactory.second->isBusy();

		fetchRequest.eventTypes.push_back(eventTypeAvailable);
	}

	service::schemas::FetchResponse fetchResponse = client.fetchTask(fetchRequest);

	for(const auto& signal : fetchResponse.signals) {
		auto iter = taskByTaskId.find(signal.taskId);
		if(iter == taskByTaskId.end()) {
			logger.warn << "Received a message to send a signal to an unknown task \"" << signal.taskId << "\"\n";
		}
		else if(iter->second->getTaskStatus().state == common::types::State::running) {
			if(esl::utility::String::toUpper(signal.signal) == "CANCEL") {
				iter->second->sendSignal(esl::utility::Signal("interrupt"));
				iter->second->sendSignal(esl::utility::Signal("terminate"));
				iter->second->sendSignal(esl::utility::Signal("pipe"));
			}
			else {
				iter->second->sendSignal(esl::utility::Signal(signal.signal));
			}
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

			std::unique_ptr<Task> task = iter->second->createTask(notifyCV, notifyMutex, settings);
			if(!task) {
				TaskStatus taskStatus;

				taskStatus.state = common::types::State::Type::zombie;
				taskStatus.returnCode = -1;
				taskStatus.message = "creating task failed";

				task.reset(new TaskFailed(std::move(taskStatus)));
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
