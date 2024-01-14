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
#include <batchelor/common/plugin/ConnectionFactory.h>
#include <batchelor/common/Timestamp.h>

#include <batchelor/worker/config/args/Config.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/plugin/Registry.h>
#include <esl/utility/String.h>

#include <cstring>
#include <iostream>
#include <memory>


namespace batchelor {
namespace worker {
namespace config {
namespace args {

void Config::printUsage() {
	std::cout << "batchelor-worker [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor-worker [CONNECTION OPTIONS] [EVENT OPTIONS] [--metric <key> <value>] [--max-tasks <value>] [--config-file <file>]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -N, --namespace             <namespace-id>  Specifies the used namespace for all commands.\n";
	std::cout << "  -T, --maximum-tasks-running <value>         Limits maximum number of tasks running at the same time.\n";
	std::cout << "  -I, --idle-timeout          <value>         Specifies a timeout greater than zero second.\n";
	std::cout << "                                              The worker will exit after it is idle for this duration. Valid values are:\n";
	std::cout << "                                              * <number>[ms]      e.g. 100000ms for 100.000 milliseconds.\n";
	std::cout << "                                              * <number>[s|sec]   e.g. 100s or 100sec for 100 seconds.\n";
	std::cout << "                                              * <number>[m|min]   e.g. 15m or 15min for 15 minutes.\n";
	std::cout << "                                              * <number>[h]       e.g. 1h for 1 hour.\n";
	std::cout << "                                              If this idle-timeout is not set, worker will never exit automatically by itself.\n";
	std::cout << "  -W, --worker-id             <value>         Set ID of worker to specified value.\n";
	std::cout << "  -m, --metric                <key> <value>   Specification of worker specific metrics that can be used in condition formulas.\n";
	std::cout << "  -f, --config-file           <file>          Configuration file can contain all connection parameters, provided event types and much more\n";
	std::cout << "  -s, --setting               <key> <value>   Event or connection specific setting.\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "EVENT OPTIONS:\n";
	std::cout << "  -e, --event-type           <id> <plugin>  Defines an event published as <id> at namespace <ns> and implemented by <plugin>.\n";
	std::cout << "                                            At least one event type must be specified.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the chosen plugin.\n";
	std::cout << "\n";
	std::cout << "                                            Most popular used plugin is \"exec\" with following settings:\n";
	std::cout << "                                            * maximum-tasks-running: <number>\n";
	std::cout << "                                            * metrics-policy:        allow|deny\n";
	std::cout << "                                            * metric:                <metric-id>\n";
	std::cout << "                                            * args:                  <arguments to call 'cmd'>\n";
	std::cout << "                                            * args-flag:             override|extend|fixed\n";
	std::cout << "                                            * env:                   <key=value>\n";
	std::cout << "                                            * env-flag-global:       override|extend\n";
	std::cout << "                                            * env-flag:              override|extend|fixed\n";
	std::cout << "                                            * cd:                    <working directory>\n";
	std::cout << "                                            * cd-flag:               override|fixed\n";
	std::cout << "                                            * cmd:                   <executable> (always fixed)\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "CONNECTION OPTIONS:\n";
	std::cout << "  -C, --connection       <plugin>           Defines the connection to a head server.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "                                            Most popular used plugin is \"basic\" with following settings:\n";
	std::cout << "                                            * url:           <server-url>  Defines the URL to the head server.\n";
	std::cout << "                                            * username:      <username>    If this setting is specified, basic-auth will be used.\n";
	std::cout << "                                            * password:      <password>    If this setting is specified, basic-auth will be used.\n";
	/*
	std::cout << "\n";
	std::cout << "                                            Another popular plugin is \"oidc\" with following settings:\n";
	std::cout << "                                            * url:           <server-url>  Defines the URL to the head server.\n";
	std::cout << "                                            * oidc-url:      <idp-url>     Defines the URL to the OAuth2 server, if client-id is used.\n";
	std::cout << "                                            * client-id:     <client-id>   If this setting is specified, OIDC protocol is used.\n";
	std::cout << "                                            * client-secret: <client-id>   If this setting is specified, OIDC protocol is used.\n";
	*/
}

using batchelor::common::config::args::ArgumentsException;
using esl::utility::String;

Config::Config(esl::object::Context& aContext, Procedure::Settings& aSettings, int argc, const char* argv[])
: context(aContext),
  settings(aSettings)
{
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(currentArg == "-m"  || currentArg == "--metric") {
			addMetrics(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-N"  || currentArg == "--namespace") {
			setNamespaceId(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-T"  || currentArg == "--maximum-tasks-running") {
			setMaximumTasksRunning(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-I"  || currentArg == "--idle-timeout") {
			setIdleTimeout(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-W"  || currentArg == "--worker-id") {
			setWorkerId(i+1 < argc ? argv[i+1] : nullptr);
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
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}

	setSettingState(SettingsState::none);
}

const std::vector<std::string>& Config::getConfigFiles() const noexcept {
	return configFiles;
}

void Config::setSettingState(SettingsState aSettingState) {
	if(settingState == SettingsState::connection) {
		++connectionCount;
		std::string id = "batchelor-control-connection-" + std::to_string(connectionCount);

		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::ConnectionFactory>(connection.plugin, connection.settings));
		if(settings.connectionFactoryIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of server connection with <id> = \"" + id + "\".");
		}
	}

	else if(settingState == SettingsState::event) {
		context.addObject(event.id, esl::plugin::Registry::get().create<plugin::TaskFactory>(event.type, event.settings));
		if(settings.taskFactoryIds.insert(event.id).second == false) {
			throw ArgumentsException("Multiple specification of option \"--event-type\" with <id> = \"" + event.id + "\".");
		}
	}

	settingState = aSettingState;

	event = Event();
	connection = Connection();
}

void Config::setNamespaceId(const char* namespaceId) {
	if(!settings.namespaceId.empty()) {
		throw ArgumentsException("Multiple specification of option \"--namespace\" is not allowed.");
	}
	if(!namespaceId) {
		throw ArgumentsException("Value missing of option \"--namespace\".");
	}

	setSettingState(SettingsState::event);

	settings.namespaceId = namespaceId;
}

void Config::setMaximumTasksRunning(const char* aMaximumTasksRunning) {
	if(settings.maximumTasksRunning != std::string::npos) {
		throw ArgumentsException("Multiple specification of option \"--maximum-tasks-running\" is not allowed.");
	}
	if(!aMaximumTasksRunning) {
		throw ArgumentsException("Value missing of option \"--maximum-tasks-running\".");
	}

	try {
		settings.maximumTasksRunning = String::toNumber<std::size_t>(aMaximumTasksRunning);
	}
	catch(const std::exception& e) {
		throw ArgumentsException("Value of option \"--maximum-tasks-running\" must be equal or greater than 0, but lower than " + std::string(aMaximumTasksRunning) + ". " + e.what());
	}
}

void Config::setIdleTimeout(const char* idleTimeout) {
	if(settings.idleTimeout.count() > 0) {
		throw ArgumentsException("Multiple specification of option \"--idle-timeout\" is not allowed.");
	}
	if(!idleTimeout) {
		throw ArgumentsException("Value missing of option \"--idle-timeout\".");
	}

	try {
		settings.idleTimeout = common::Timestamp::toDuration(idleTimeout);
	}
	catch(const std::exception& e) {
		throw ArgumentsException("Invalid value \"" + std::string(idleTimeout) + "\" of option \"--idle-timeout\". " + e.what());
	}
}

void Config::setWorkerId(const char* workerId) {
	if(!settings.workerId.empty()) {
		throw ArgumentsException("Multiple specification of option \"--worker-id\" is not allowed.");
	}
	if(!workerId) {
		throw ArgumentsException("Value missing of option \"--worker-id\".");
	}

	settings.workerId = workerId;

	if(settings.workerId.empty()) {
		throw ArgumentsException("Invalid value \"\" for option \"--worker-id\".");
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

void Config::addSettings(const char* key, const char* value) {
	if(!key) {
		throw ArgumentsException("Key missing of option \"--setting\".");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--setting\".");
	}

	switch(settingState) {
	case SettingsState::none:
		throw ArgumentsException("Option \"--setting\" is invalid, because there is no previous option \"--connection\" or \"--event-type\".");
	case SettingsState::event:
		event.settings.emplace_back(key, value);
		break;
	case SettingsState::connection:
		connection.settings.emplace_back(key, value);
	}
}

void Config::addEvent(const char* id, const char* plugin) {
	if(!id) {
		throw ArgumentsException("Id missing of option \"--event-type\".");
	}
	if(!plugin) {
		throw ArgumentsException("Plugin missing of option \"--event-type\".");
	}

	setSettingState(SettingsState::event);

	event.id = id;
	event.type = plugin;
}

void Config::addConnection(const char* value) {
	if(!value) {
		throw ArgumentsException("Plugin-value missing of option \"--connection\".");
	}

	setSettingState(SettingsState::connection);
	connection.plugin = value;
}

} /* namespace args */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
