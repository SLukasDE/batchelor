#include <batchelor/worker/cli/Logger.h>
#include <batchelor/worker/cli/Main.h>

#include <esl/logging/Logging.h>
#include <esl/plugin/Registry.h>
#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/system/Stacktrace.h>

#include <eslx/Plugin.h>

#include <iostream>
#include <stdexcept>

batchelor::worker::cli::Logger logger("batchelor::worker::cli");

#if 1
class RegistryGuard {
public:
	~RegistryGuard() {
		esl::plugin::Registry::cleanup();
	}
};
#endif

int main(int argc, const char* argv[]) {
	int rc = -1;
#if 1
	RegistryGuard registry;
#endif

	try {
		eslx::Plugin::install(esl::plugin::Registry::get(), nullptr);
	    esl::system::Stacktrace::init("eslx/system/Stacktrace", {});
		esl::logging::Logging::initWithFile("logger.xml");

		batchelor::worker::cli::Main();

		rc = 0;
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
#if 0
	esl::plugin::Registry::cleanup();
#endif

	return rc;
}
