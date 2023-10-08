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

#include <batchelor/head/jerrycli/config/args/Config.h>

#include <batchelor/common/config/args/ArgumentsException.h>
#include <batchelor/common/String.h>

#include <iostream>

namespace batchelor {
namespace head {
namespace jerrycli {
namespace config {
namespace args {

using batchelor::common::config::args::ArgumentsException;
using batchelor::common::String;

Config::Config(Main::Settings& aSettings, int argc, const char* argv[])
: settings(aSettings)
{
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(currentArg == "-P"  || currentArg == "--port") {
			setPort(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-T"  || currentArg == "--max-threads") {
			setMaximumThreads(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-f"  || currentArg == "--config-file") {
			addConfigFile(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-u"  || currentArg == "--username") {
			setUsername(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-p"  || currentArg == "--password") {
			setPassword(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-S"  || currentArg == "--https") {
			settings.isHttps = true;
		}
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}
}

void Config::printUsage() {
	std::cout << "batchelor-head [OPTIONS]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -P, --port             <port-number>    Port to listen. Default value is 8080\n";
	std::cout << "  -T, --max-threads      <number>         Limits maximum number of threads to handle connections at the same time. Default value is 4.\n";
	std::cout << "  -f, --config-file      <file>           Configuration file\n";
	std::cout << "  -u, --username         <username>       If <username> is specified, basic-auth is used\n";
	std::cout << "  -p, --password         <password>       If <password> is specified, basic-auth is used\n";
	std::cout << "  -S, --https                             Enables HTTPS.\n";
}

const std::vector<std::string>& Config::getConfigFiles() const noexcept {
	return configFiles;
}

void Config::setPort(const char* port) {
	if(settings.port != 0) {
		throw ArgumentsException("Multiple specification of option \"--port\" is not allowed.");
	}
	if(!port) {
		throw ArgumentsException("Value missing of option \"--port\".");
	}

	try {
		settings.port = String::toNumber<unsigned short>(port);
	}
	catch(const std::exception& e) {
		throw ArgumentsException("Value of option \"--port\" must be equal or greater than 0, but lower than " + std::string(port) + ". " + e.what());
	}
	if(settings.port == 0) {
		throw ArgumentsException("Value of option \"--port\" must be greater than 0.");
	}
}

void Config::setMaximumThreads(const char* threads) {
	if(settings.threads != 0) {
		throw ArgumentsException("Multiple specification of option \"--max-threads\" is not allowed.");
	}
	if(!threads) {
		throw ArgumentsException("Value missing of option \"--max-tasks\".");
	}

	try {
		settings.threads = String::toNumber<unsigned short>(threads);
	}
	catch(const std::exception& e) {
		throw ArgumentsException("Value of option \"--max-tasks\" must be equal or greater than 0, but lower than " + std::string(threads) + ". " + e.what());
	}
	if(settings.threads == 0) {
		throw ArgumentsException("Value of option \"--max-tasks\" must be greater than 0.");
	}
}

void Config::addConfigFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--config-file\".");
	}
	configFiles.push_back(value);
}

void Config::setUsername(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--username\".");
	}
	settings.username = value;
}

void Config::setPassword(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--password\".");
	}
	settings.password = value;
}

} /* namespace args */
} /* namespace config */
} /* namespace jerrycli */
} /* namespace head */
} /* namespace batchelor */
