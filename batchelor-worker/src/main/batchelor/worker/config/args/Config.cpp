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
#include <batchelor/common/config/Server.h>
#include <batchelor/common/String.h>

#include <batchelor/worker/config/args/Config.h>

#include <cstring>
#include <iostream>

namespace batchelor {
namespace worker {
namespace config {
namespace args {

using batchelor::common::config::args::ArgumentsException;
using batchelor::common::String;

Config::Config(Main::Settings& aSettings, int argc, const char* argv[])
: settings(aSettings)
{
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(currentArg == "-m"  || currentArg == "--metric") {
			addMetrics(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-T"  || currentArg == "--max-tasks") {
			setMaximumTasksRunning(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-f"  || currentArg == "--config-file") {
			addConfigFile(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-s"  || currentArg == "--setting") {
			addSettings(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-e"  || currentArg == "--event-type") {
			addEvent(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-C"  || currentArg == "--connection") {
			addConnection(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-U"  || currentArg == "--server-url") {
			addURL(i+1 < argc ? argv[i+1] : nullptr);
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
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}
}

void Config::printUsage() {
	std::cout << "batchelor-worker [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor-worker [CONNECTION OPTIONS] [EVENT OPTIONS] [--metric <key> <value>] [--max-tasks <value>] [--config-file <file>]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -T, --max-tasks        <value>          Limits maximum number of tasks running at the same time.\n";
	std::cout << "  -m, --metric           <key> <value>    Specification of worker specific metrics that can be used in condition formulas.\n";
	std::cout << "  -f, --config-file      <file>           Configuration file can contain all connection parameters, provided event types and much more\n";
	std::cout << "  -s, --setting          <key> <value>    Event or connection specific setting.\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "EVENT OPTIONS:\n";
	std::cout << "  -e, --event-type       <id> <plugin>    Defines an event published as <id> and implemented by <plugin>. At least one event type must be specified.\n";
	std::cout << "                                          Subsequent settings specified by \"--setting\" are specific to the chosen plugin.\n";
	std::cout << "                                          Most popular used plugin is \"exec\" with following settings:\n";
	std::cout << "                                          * maximum-tasks-running: <number>\n";
	std::cout << "                                          * metrics-policy:        allow|deny\n";
	std::cout << "                                          * metric:                <metric-id>\n";
	std::cout << "                                          * args:                  <arguments to call 'cmd'>\n";
	std::cout << "                                          * args-flags:            override|extend|fixed\n";
	std::cout << "                                          * env:                   <key=value>\n";
	std::cout << "                                          * env-flag-global:       override|extend\n";
	std::cout << "                                          * env-flag:              override|extend|fixed\n";
	std::cout << "                                          * cd:                    <working directory>\n";
	std::cout << "                                          * cd-flag:               override|fixed\n";
	std::cout << "                                          * cmd:                   <executable> (always fixed)\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "CONNECTION OPTIONS:\n";
	std::cout << "  -C, --connection       <plugin>         Defines a head server to ask for tasks to process. At least one server-url must be specified.\n";
	std::cout << "                                          Argument <plugin> is reserved for future features and must be set by any value, even if it has no effect.\n";
	std::cout << "                                          Subsequent settings specified by \"--setting\" are specific to the plugin that is internally used.\n";
	std::cout << "  -U, --server-url       <server-url>     Defines the URL to the head server with the previously specified connection plugin.\n";
	std::cout << "  -u, --username         <username>       If <username> is specified, basic-auth is used for the previously specified connection plugin.\n";
	std::cout << "  -p, --password         <password>       If <password> is specified, basic-auth is used for the previously specified connection plugin.\n";
}

const std::vector<std::string>& Config::getConfigFiles() const noexcept {
	return configFiles;
}

void Config::setMaximumTasksRunning(const char* aMaximumTasksRunning) {
	if(settings.maximumTasksRunning != std::string::npos) {
		throw ArgumentsException("Multiple specification of option \"--max-tasks\" is not allowed.");
	}
	if(!aMaximumTasksRunning) {
		throw ArgumentsException("Value missing of option \"--max-tasks\".");
	}

	try {
		settings.maximumTasksRunning = String::toNumber<std::size_t>(aMaximumTasksRunning);
	}
	catch(const std::exception& e) {
		throw ArgumentsException("Value of option \"--max-tasks\" must be equal or greater than 0, but lower than " + std::string(aMaximumTasksRunning) + ". " + e.what());
	}
}

void Config::addMetrics(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--metric\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--metric\".");
	}
	settings.metrics.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
}

void Config::addConfigFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--config-file\".");
	}
	configFiles.push_back(value);
}

void Config::addSettings(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--setting\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--setting\".");
	}

	switch(settingState) {
	case SettingsState::none:
		throw ArgumentsException("Option \"--setting\" is invalid, because there is no previous option \"--connection\" or \"--event-type\".");
	case SettingsState::event:
		if(settings.events.empty()) {
			throw ArgumentsException("Internal error: empty event list.");
		}
		settings.events[settings.events.size()-1].settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
		break;
	case SettingsState::connection:
		if(settings.servers.empty()) {
			throw ArgumentsException("Internal error: empty server list.");
		}
		settings.servers[settings.servers.size()-1].settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
		break;
	}
}

void Config::addEvent(const char* aId, const char* aPlugin) {
	if(!aId) {
		throw ArgumentsException("Id missing of option \"--event-type\".");
	}
	if(!aPlugin) {
		throw ArgumentsException("Plugin missing of option \"--event-type\".");
	}

	Main::Settings::Event event;
	event.id = aId;
	event.type = aPlugin;
	settings.events.push_back(event);
	settingState = SettingsState::event;
}

void Config::addConnection(const char* value) {
	if(!value) {
		throw ArgumentsException("Plugin-value missing of option \"--connection\".");
	}

	common::config::Server server;
	server.plugin = value;
	settings.servers.push_back(server);
	settingState = SettingsState::connection;
}

void Config::setUsername(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--username\".");
	}

	if(settings.servers.empty()) {
		if(settingState == SettingsState::connection) {
			throw ArgumentsException("Internal error (username): empty server list.");
		}
		throw ArgumentsException("Connection missing for option \"--username\".");
	}
	settings.servers[settings.servers.size()-1].username = value;
}

void Config::setPassword(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--password\".");
	}

	if(settings.servers.empty()) {
		if(settingState == SettingsState::connection) {
			throw ArgumentsException("Internal error (password): empty server list.");
		}
		throw ArgumentsException("Connection missing for option \"--password\".");
	}
	settings.servers[settings.servers.size()-1].username = value;
}

void Config::addURL(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--server-url\".");
	}

	if(settings.servers.empty()) {
		if(settingState == SettingsState::connection) {
			throw ArgumentsException("Internal error (server-url): empty server list.");
		}
		throw ArgumentsException("Connection missing for option \"--server-url\".");
	}
	settings.servers[settings.servers.size()-1].url = value;
}

} /* namespace args */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
