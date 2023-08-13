#include <batchelor/common/ArgumentsException.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/Main.h>
#include <batchelor/worker/Options.h>

#include <esl/logging/Logging.h>
#include <esl/plugin/Registry.h>
#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/system/Stacktrace.h>

#include <eslx/Plugin.h>

#include <iostream>
#include <stdexcept>

batchelor::worker::Logger logger("batchelor::worker");

int main(int argc, const char* argv[]) {
	using namespace batchelor::worker;
	using namespace batchelor::common;

	int rc = -1;

	try {
		Options options(argc, argv);

		eslx::Plugin::install(esl::plugin::Registry::get(), nullptr);
	    esl::system::Stacktrace::init("eslx/system/Stacktrace", {});
		esl::logging::Logging::initWithFile("logger.xml");

		Main main{ options };

		rc = main.getReturnCode();
	}
    catch(const ArgumentsException& e) {
    	std::cerr << e.what() << "\n";
		Options::printUsage();
    }
    catch(const esl::plugin::exception::PluginNotFound& e) {
        std::cerr << "Plugin not found exception occurred: " << e.what() << "\n";
        const esl::plugin::Registry::BasePlugins& basePlugins = esl::plugin::Registry::get().getPlugins(e.getTypeIndex());
        if(basePlugins.empty()) {
            std::cerr << "No implementations available.\n";
        }
        else {
            std::cerr << "Implementations available:\n";
            for(const auto& basePlugin : basePlugins) {
                std::cerr << "- " << basePlugin.first << "\n";
            }
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << "\n";
    }
    catch(...) {
        std::cerr << "Unknown exception occurred.\n";
    }

	esl::plugin::Registry::cleanup();

	return rc;
}
