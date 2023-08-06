#include <batchelor/control/Main.h>
#include <batchelor/control/Logger.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

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
		waitTask();
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
	auto httpConnection = createConnection();
	service::client::Service client(*httpConnection);

	service::schemas::RunRequest runRequest;
	runRequest.eventType = options.getEventType();
	runRequest.priority = options.getPriority() < 0 ? 0 : options.getPriority();
	for(const auto& setting : options.getSettings()) {
		runRequest.settings.push_back(service::schemas::Setting::make(setting.first, setting.second));
	}
	runRequest.condition = options.getCondition().empty() ? "${TRUE}" : options.getCondition();

	service::schemas::RunResponse runResponse = client.runTask(runRequest);
	logger.info << "Task ID: \"" << runResponse.jobId << "\"\n";
	logger.info << "Message: \"" << runResponse.message << "\"\n";
}

void Main::waitTask() {

}

void Main::signalTask() {

}

void Main::showTask() {
	auto httpConnection = createConnection();
	service::client::Service client(*httpConnection);

	std::unique_ptr<service::schemas::JobStatusHead> taskHead = client.getTask(options.getTaskId());
	if(taskHead) {
		logger.info << "Task ID    : \"" << taskHead->runConfiguration.jobId << "\"\n";
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

std::unique_ptr<esl::com::http::client::Connection> Main::createConnection() {
	auto httpConnection = getConnectionFactory().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

esl::com::http::client::ConnectionFactory& Main::getConnectionFactory() {
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
