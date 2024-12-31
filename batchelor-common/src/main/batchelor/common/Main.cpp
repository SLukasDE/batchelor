/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <batchelor/common/config/args/ArgumentsException.h>
#include <batchelor/common/Main.h>

#include <openesl/Plugin.h>

#include <esl/crypto/GTXKeyStore.h>
#include <esl/monitoring/LogbookLogging.h>
#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>
#include <esl/system/ZSSignalManager.h>
#include <esl/system/ZSStacktraceFactory.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace batchelor {
namespace common {

Main::Main(int(*run)(int, const char**), void(*printUsage)(), int argc, const char* argv[])
{
	rc = -1;

    struct RegistryGuard {
        ~RegistryGuard() {
            esl::plugin::Registry::cleanup();
        }
    } registryGuard;

	try {
		esl::plugin::Registry& registry(esl::plugin::Registry::get());
        openesl::Plugin::install(registry, nullptr);
		registry.setObject(esl::crypto::GTXKeyStore::createNative());
		//registry.setObject(esl::system::ZSStacktraceFactory::createNative());
#if 1
		{
			esl::system::ZSSignalManager::Settings aSettings;
			aSettings.isThreaded = true;
			registry.setObject(esl::system::ZSSignalManager::createNative(aSettings));
		}
#else
		registry.setObject(esl::system::ZSSignalManager::createNative(esl::system::ZSSignalManager::Settings(std::vector<std::pair<std::string, std::string>>({{"is-threaded", "true"}}))));
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
