#include <batchelor/common/config/args/ArgumentsException.h>
#include <batchelor/common/Main.h>

#include <esl/Plugin.h>

#include <esl/crypto/GTXKeyStore.h>
#include <esl/monitoring/LogbookLogging.h>
#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>
#include <esl/system/DefaultSignalManager.h>
#include <esl/system/DefaultStacktraceFactory.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace batchelor {
namespace common {

Main::Main(int(*run)(int, const char**), void(*printUsage)(), int argc, const char* argv[])
{
	rc = -1;

	try {
		esl::plugin::Registry& registry(esl::plugin::Registry::get());
		esl::Plugin::install(registry, nullptr);
		registry.setObject(esl::crypto::GTXKeyStore::createNative());
		registry.setObject(esl::system::DefaultStacktraceFactory::createNative());
#if 1
		{
			esl::system::DefaultSignalManager::Settings aSettings;
			aSettings.isThreaded = true;
			registry.setObject(esl::system::DefaultSignalManager::createNative(aSettings));
		}
#else
		registry.setObject(esl::system::DefaultSignalManager::createNative(esl::system::DefaultSignalManager::Settings(std::vector<std::pair<std::string, std::string>>({{"is-threaded", "true"}}))));
#endif

		if(!std::ifstream("logger.xml").fail()) {
			auto logging = esl::monitoring::LogbookLogging::createNative();
			logging->addFile("logger.xml");
			registry.setObject(std::move(logging));
		}

		rc = run(argc, argv);
	}
    catch(const config::args::ArgumentsException& e) {
    	std::cerr << e.what() << "\n";
    	printUsage();
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
}

Main::~Main() {
	esl::plugin::Registry::cleanup();
}

int Main::getReturnCode() const noexcept {
	return rc;
}

} /* namespace common */
} /* namespace batchelor */
