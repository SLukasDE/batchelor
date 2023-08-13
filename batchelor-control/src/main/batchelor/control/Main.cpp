#include <batchelor/control/Main.h>
#include <batchelor/control/Logger.h>

#include <batchelor/common/types/State.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/TaskStatusHead.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace control {
namespace {
Logger logger("batchelor::control::Main");
}

Main::Main(const Options& aOptions)
: options(aOptions),
  url("http://localhost:8080")
{
	switch(options.getCommand()) {
	case Command::help:
		Options::printUsage();
		break;
	case Command::sendEvent:
		sendEvent();
		break;
	case Command::waitTask:
		waitTask(options.getTaskId());
		break;
	case Command::signalTask:
		signalTask();
		break;
	case Command::showTask:
		showTask();
		break;
	case Command::showTasks:
		showTasks();
		break;
	}
}

void Main::sendEvent() {
	service::schemas::RunResponse runResponse;
	{
		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		service::schemas::RunRequest runRequest;
		runRequest.eventType = options.getEventType();
		runRequest.priority = options.getPriority() < 0 ? 0 : options.getPriority();
		for(const auto& setting : options.getSettings()) {
			runRequest.settings.push_back(service::schemas::Setting::make(setting.first, setting.second));
		}
		runRequest.condition = options.getCondition().empty() ? "${TRUE}" : options.getCondition();

		runResponse = client.runTask(runRequest);
		logger.info << "Task ID    : \"" << runResponse.taskId << "\"\n";
	}

	if(options.getWait()) {
		logger.info << "-----------------\n";
		waitTask(runResponse.taskId);
	}
}

void printStatus(const service::schemas::TaskStatusHead& taskStatus) {
	common::types::State::Type state = common::types::State::toState(taskStatus.state);

	logger.info << "State      : \"" << taskStatus.state << "\"\n";
	logger.info << "Created TS : \"" << taskStatus.tsCreated << "\"\n";
	switch(state) {
	case common::types::State::queued:
		break;
	case common::types::State::running:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		break;
	case common::types::State::done:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskStatus.tsFinished << "\"\n";
		logger.info << "Return code: \"" << taskStatus.returnCode << "\"\n";
		break;
	case common::types::State::signaled:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskStatus.tsFinished << "\"\n";
		logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		break;
	case common::types::State::zombie:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		break;
	}
}

void Main::waitTask(const std::string& taskId) {
	service::schemas::TaskStatusHead oldTaskStatus;
	{
		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(taskId);
		if(!taskStatus) {
			return;
		}

		rc = taskStatus->returnCode;

		printStatus(*taskStatus);

		oldTaskStatus = *taskStatus;
	}

	while(true) {
		{
			common::types::State::Type state = common::types::State::toState(oldTaskStatus.state);
			if(state == common::types::State::done || state == common::types::State::signaled || state == common::types::State::zombie) {
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(taskId);
		if(!taskStatus) {
			return;
		}

		if(oldTaskStatus.state == taskStatus->state) {
			continue;
		}

		rc = taskStatus->returnCode;

		logger.info << "-----------------\n";
		printStatus(*taskStatus);

		oldTaskStatus = *taskStatus;
	}
}

void Main::signalTask() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	client.sendSignal(options.getTaskId(), options.getSignal());
}

void Main::showTask() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::unique_ptr<service::schemas::TaskStatusHead> taskHead = client.getTask(options.getTaskId());
	if(taskHead) {
		logger.info << "Task ID    : \"" << taskHead->runConfiguration.taskId << "\"\n";
		logger.info << "Event type : \"" << taskHead->runConfiguration.eventType << "\"\n";
		if(taskHead->runConfiguration.settings.empty()) {
			logger.info << "Settings   : (empty)\n";
		}
		else {
			logger.info << "Settings   :\n";
			for(std::size_t i = 0; i<taskHead->runConfiguration.settings.size(); ++i) {
				logger.info << "  [" << (i+1) << "]: \"" << taskHead->runConfiguration.settings[i].key << "\" = \"" << taskHead->runConfiguration.settings[i].value << "\"\n";
			}

		}
		logger.info << "Metrics    :\n";
		for(std::size_t i = 0; i<taskHead->metrics.size(); ++i) {
			logger.info << "  [" << (i+1) << "]: \"" << taskHead->metrics[i].key << "\" = \"" << taskHead->runConfiguration.settings[i].value << "\"\n";
		}
		logger.info << "State      : \"" << taskHead->state << "\"\n";
		logger.info << "Return code: \"" << taskHead->returnCode << "\"\n";
		logger.info << "Message    : \"" << taskHead->message << "\"\n";
		logger.info << "Created TS : \"" << taskHead->tsCreated << "\"\n";
		logger.info << "Running TS : \"" << taskHead->tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskHead->tsFinished << "\"\n";
		logger.info << "Heartbeat  : \"" << taskHead->tsLastHeartBeat << "\"\n";
	}
}

void Main::showTasks() {

}

int Main::getReturnCode() const {
	return rc;
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

} /* namespace control */
} /* namespace batchelor */
