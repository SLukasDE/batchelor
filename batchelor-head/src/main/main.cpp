#include <batchelor/head/Logger.h>
#include <batchelor/head/Plugin.h>
#include <batchelor/head/RequestHandler.h>

#include <eslx/Plugin.h>
#include <eslx/object/JerryProcessingContext.h>

#include <jerry4esl/ExceptionHandler.h>
#include <jerry4esl/engine/http/Server.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>



extern const std::string batchelorHeadVersionStr;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
const std::string batchelorHeadVersionStr = STRINGIFY(TRANSFORMER_ARTEFACT_VERSION);

batchelor::head::Logger logger("batchelor::head");

int main(int argc, const char *argv[]) {
	int returnCode = 0;

	try {
		std::cout << "batchelor-head version " << batchelorHeadVersionStr << std::endl;

		eslx::Plugin::install(esl::plugin::Registry::get(), nullptr);
		batchelor::head::Plugin::install(esl::plugin::Registry::get(), nullptr);

		esl::system::Stacktrace::init("eslx/system/Stacktrace", {});

		eslx::object::JerryProcessingContext::Settings settings;

		//settings.setSignalHandler(true);
		settings.addStopSignal(esl::utility::Signal("interrupt"));
		settings.addStopSignal(esl::utility::Signal("terminate"));
		settings.addStopSignal(esl::utility::Signal("pipe"));
		settings.setTerminateCounter(2);
		settings.setVerbose(true);

		eslx::object::JerryProcessingContext context(std::move(settings));

//		context.addFile(jerry4esl::config::Flags::get().serverConfigFile);
		context.addData(
R"BATCHELOR-DATA(
<context>
  <database id="my-db" implementation="eslx/database/SQLiteConnectionFactory">
    <!-- parameter key="URI" value="file:my.db?mode=rw"/ -->
    <parameter key="URI" value="file:test?mode=memory"/>
  </database>
</context>
)BATCHELOR-DATA");

		{
			std::unique_ptr<jerry4esl::engine::http::Server> server(new jerry4esl::engine::http::Server("", {{{"threads"}, {"4"}}, {{"port"}, {"8080"}}}, false));
			server->add(std::move(batchelor::head::RequestHandler::create({{{"db-connection-factory"},{"my-db"}}})));
			server->setParentObjectContext(&context);
			context.add(std::move(server));
		}

		/* ************************* *
		 * initialize global objects *
		 * ************************* */
		logger.info << "Initialize objects ...\n";
		context.initialize();
		logger.info << "Initialization done.\n";

		logger.debug << "Start all processes...\n";
		returnCode = context.main();
	}
	catch(...) {
		jerry4esl::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);
    	returnCode = -1;

		if(esl::logging::Logging::get()) {
			std::stringstream strStream;
			esl::logging::Logging::get()->flush(&strStream);
			std::cerr << strStream.str();
		}
	}

	esl::plugin::Registry::cleanup();

	return returnCode;
}
