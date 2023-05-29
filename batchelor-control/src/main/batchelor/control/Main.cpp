#include <batchelor/control/Main.h>
#include <batchelor/control/Logger.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>


namespace batchelor {
namespace control {
namespace {
Logger logger("batchelor::control::Main");
}

Main::Main(int argc, const char* argv[]) {
	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory = esl::plugin::Registry::get().create<esl::com::http::client::ConnectionFactory>(
			"eslx/com/http/client/ConnectionFactory", {
					{"url", "http://localhost:8080"}
			});
	auto httpConnection = httpConnectionFactory->createConnection();
	service::client::Service client(*httpConnection);

	service::schemas::RunRequest runRequest;
	runRequest.batchId = "bestoptxl";
	runRequest.priority = 0;
	runRequest.arguments.emplace_back("--WW_BEREICH=0");
	runRequest.envVars.push_back(service::schemas::Setting::make("key", "value"));
	runRequest.settings.push_back(service::schemas::Setting::make("key", "value"));

	service::schemas::RunResponse runResponse = client.runBatch(runRequest);
	logger.info << "Job ID : \"" << runResponse.jobId << "\"\n";
	logger.info << "Message: \"" << runResponse.message << "\"\n";
}

} /* namespace control */
} /* namespace batchelor */
