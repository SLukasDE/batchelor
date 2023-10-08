/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#include <batchelor/head/jerrycli/config/args/Config.h>
#include <batchelor/head/jerrycli/config/xml/Config.h>
#include <batchelor/head/jerrycli/Logger.h>
#include <batchelor/head/jerrycli/Main.h>

#include <esl/system/Stacktrace.h>

#include <eslx/Plugin.h>

#if 0
#include <jerry4esl/ExceptionHandler.h>
#endif

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <fstream>



extern const std::string artefactVersionStr;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
const std::string artefactVersionStr = STRINGIFY(TRANSFORMER_ARTEFACT_VERSION);

batchelor::head::jerrycli::Logger logger("batchelor::head::jerrycli");

int main(int argc, const char* argv[]) {
	using batchelor::common::config::args::ArgumentsException;
	using batchelor::head::jerrycli::Main;
	using ArgsConfig = batchelor::head::jerrycli::config::args::Config;
	using XmlConfig = batchelor::head::jerrycli::config::xml::Config;

	std::cout << "batchelor-head-jerrycli version " << artefactVersionStr << std::endl;

	int rc = -1;

	struct RegistryGuard {
		~RegistryGuard() { esl::plugin::Registry::cleanup(); }
	} registryGuard;

	try {
		eslx::Plugin::install(esl::plugin::Registry::get(), nullptr);
		esl::system::Stacktrace::init("eslx/system/Stacktrace", {});
	    {
	    	std::ifstream loggerFile("logger.xml");
	    	if(!loggerFile.fail()) {
	    		esl::logging::Logging::initWithFile("logger.xml");
    	    }
	    }

		Main::Settings settings;
		ArgsConfig argsConfig(settings, argc, argv);

		/* load additional data from config files */
		for(const auto& configFile : argsConfig.getConfigFiles()) {
			XmlConfig(settings, configFile);
		}

		Main main{ settings };

		rc = main.getReturnCode();
	}
#if 0
	catch(...) {
		jerry4esl::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);

		if(esl::logging::Logging::get()) {
			std::stringstream strStream;
			esl::logging::Logging::get()->flush(&strStream);
			std::cerr << strStream.str();
		}
	}
#else
    catch(const ArgumentsException& e) {
    	std::cerr << e.what() << "\n";
    	ArgsConfig::printUsage();
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
#endif

	return rc;
}
