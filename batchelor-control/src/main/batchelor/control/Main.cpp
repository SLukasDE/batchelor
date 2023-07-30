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
	case Command::cancelTask:
		cancelTask();
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
	runRequest.eventType = "bestoptxl";
	runRequest.priority = 0;
	runRequest.settings.push_back(service::schemas::Setting::make("env", "DISPLAY=0"));
	runRequest.settings.push_back(service::schemas::Setting::make("env", "TMP_DIR=/tmp"));
	runRequest.settings.push_back(service::schemas::Setting::make("args", "--WW_BEREICH=0 --propertyId=Bla --propertyFile=/wxx/secret/property.cfg"));
	runRequest.condition = "${TRUE}";

	service::schemas::RunResponse runResponse = client.runJob(runRequest);
	logger.info << "Job ID : \"" << runResponse.jobId << "\"\n";
	logger.info << "Message: \"" << runResponse.message << "\"\n";
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

void Main::waitTask() {

}

void Main::cancelTask() {

}

void Main::signalTask() {

}

void Main::showTask() {

}

void Main::showTasks() {

}

} /* namespace control */
} /* namespace batchelor */
