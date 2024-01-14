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

#include <batchelor/head/config/args/Config.h>
#include <batchelor/head/plugin/Observer.h>
#include <batchelor/head/plugin/Socket.h>

#include <batchelor/common/config/args/ArgumentsException.h>

#include <sqlite4esl/database/ConnectionFactory.h>

#include <esl/database/ConnectionFactory.h>
#include <esl/database/SQLiteConnectionFactory.h>
#include <esl/plugin/Registry.h>
#include <esl/utility/String.h>

#include <iostream>

namespace batchelor {
namespace head {
namespace config {
namespace args {

void Config::printUsage() {
	std::cout << "batchelor-head [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor-head [DATABASE] [OBSERVER OPTIONS] [SOCKET OPTIONS] [--config-file <file>]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -f, --config-file      <file>             Configuration file\n";
	std::cout << "\n";
	std::cout << "  -s, --setting          <key> <value>      Observer or socket specific setting.\n";
	std::cout << "\n";
	std::cout << "  -c, --certificate      <host> <key> <crt> Defines a certificate to use for hostname <host>. The certificate is repesented as\n";
	std::cout << "                                            two files, the key-file, specified by <key> and the cert-file, specified by <crt>.\n";
	std::cout << "\n";
	std::cout << "  -G, --group            <grp> <ns> <role>  Defines a group <grp> that is provided with a role <role> at namespace <ns>.\n";
	std::cout << "                                            To add several role-to-namespace settings to a group, just repeat the definition several\n";
	std::cout << "                                            times with the same group <grp>, but differen values for <role> and <ns>.\n";
	std::cout << "                                            Later, users can join a group. This simplifies the case to equipped many users with\n";
	std::cout << "                                            the same bunch of namespace specific roles.\n";
	std::cout << "                                            Following roles are availabe:\n";
	std::cout << "                                            * read-only: A controller needs this role to watch information of tasks.\n";
	std::cout << "                                            * execute:   A controller needs this role to send events-\n";
	std::cout << "                                            * worker:    A worker needs this role to provide event types.\n";
	std::cout << "\n";
	std::cout << "  -U, --user             <user> <grp>       Defines a user <user> as member of a group <grp>, that is provided with a bunch of\n";
	std::cout << "                                            namespace specific roles.\n";
	std::cout << "\n";
	std::cout << "  -B, --basic-auth       <user> <pw>        Defines a password for a user <user> used if basic-authentication is used over https.\n";
	std::cout << "                                            The password is specified in <pw> that has format \"<encryption>:<value>\".\n";
	std::cout << "                                            There are several values available for <encryption>:\n";
	std::cout << "                                            * plan:<value>      Defines the password as plain text in <value>.\n";
	std::cout << "\n";
//	std::cout << "  -D, --database         <plugin>           Defines a database to store status data.\n";
//	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
//	std::cout << "\n";
	std::cout << "  -O, --observer         <plugin>           Defines an observer to listen on events.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "  -S, --socket           <plugin>           Defines a socket to listen for requests.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "                                            Most popular used plugin is \"http\" with following settings:\n";
	std::cout << "                                            * port:          <number>      Defines the port number to listen for HTTP requests.\n";
	std::cout << "                                            * threads:       <number>      Defines the number of requests to handle parallel.\n";
	std::cout << "                                            * https:         <true|false>  Defines if http or https is used.\n";
}

using batchelor::common::config::args::ArgumentsException;
using esl::utility::String;

Config::Config(esl::object::Context& aContext, Procedure::Settings& aSettings, int argc, const char* argv[])
: context(aContext),
  settings(aSettings)
{
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(currentArg == "-f"  || currentArg == "--config-file") {
			addConfigFile(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-s"  || currentArg == "--setting") {
			addSettings(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-c"  || currentArg == "--certificate") {
			addCertificate(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr, i+3 < argc ? argv[i+3] : nullptr);
			i = i+3;
		}
		else if(currentArg == "-G"  || currentArg == "--group") {
			addGroup(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr, i+3 < argc ? argv[i+3] : nullptr);
			i = i+3;
		}
		else if(currentArg == "-U"  || currentArg == "--user") {
			addUser(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-B"  || currentArg == "--basic-auth") {
			addBasicAuth(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-D"  || currentArg == "--database") {
			addDatabase(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-O"  || currentArg == "--observer") {
			addObserver(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-S"  || currentArg == "--socket") {
			addSocket(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}

	setSettingState(SettingsState::none);

	for(auto& user : settings.users) {
		const auto& groups = groupsByUser[user.first];
		for(const auto& group : groups) {
			const auto& namespacesRoles = namespacesRolesByGroup[group];
			for(const auto& namespaceRole : namespacesRoles) {
				user.second.rolesByNamespace[namespaceRole.first].insert(namespaceRole.second);
			}
		}
	}
}

const std::vector<std::string>& Config::getConfigFiles() const noexcept {
	return configFiles;
}

void Config::setSettingState(SettingsState aSettingState) {
	if(settingState == SettingsState::database) {
		context.addObject("batchelor-db", esl::plugin::Registry::get().create<esl::database::ConnectionFactory>(database->implementation, database->settings));
	}
	else if(settingState == SettingsState::observer) {
		++observerCount;
		std::string id = "batchelor-head-observer-" + std::to_string(observerCount);

		context.addObject(id, esl::plugin::Registry::get().create<plugin::Observer>(observer.implementation, observer.settings));
		if(settings.observerIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of observer with <id> = \"" + id + "\".");
		}
	}

	else if(settingState == SettingsState::socket) {
		++socketCount;
		std::string id = "batchelor-head-socket-" + std::to_string(socketCount);

		context.addObject(id, esl::plugin::Registry::get().create<plugin::Socket>(socket.implementation, socket.settings));
		if(settings.socketIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of socket with <id> = \"" + id + "\".");
		}
	}

	settingState = aSettingState;

	observer = Observer();
	socket = Socket();

	if(settingState == SettingsState::database) {
		if(database) {
			throw ArgumentsException("Multiple definition of database.");
		}
		database.reset(new Database());
	}
	else if(settingState == SettingsState::none && !database) {
		context.addObject("batchelor-db", std::unique_ptr<esl::database::ConnectionFactory>(new sqlite4esl::database::ConnectionFactory(esl::database::SQLiteConnectionFactory::Settings({
			//{{"URI"}, {":memory:"}}
			{{"URI"}, {"file:test?mode=memory"}}
			//{{"URI"}, {"file::memory:?mode=rw"}}
		}))));
	}
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
		throw ArgumentsException("Option \"--setting\" is invalid, because there is no previous option \"--observer\" or \"--socket\".");
	case SettingsState::database:
		database->settings.emplace_back(key, value);
		break;
	case SettingsState::observer:
		observer.settings.emplace_back(key, value);
		break;
	case SettingsState::socket:
		socket.settings.emplace_back(key, value);
		break;
	}
}

void Config::addCertificate(const char* hostName, const char* keyFile, const char* certFile) {

}

void Config::addGroup(const char* group, const char* namespaceId, const char* roleStr1) {
	if(!group) {
		throw ArgumentsException("Group name missing of option \"--group\".");
	}

	if(!namespaceId) {
		throw ArgumentsException("Namespace missing of option \"--group\".");
	}

	if(!roleStr1) {
		throw ArgumentsException("Role missing of option \"--group\".");
	}
	std::string roleStr2(roleStr1);
	Procedure::Settings::Role role = Procedure::Settings::Role::readOnly;
	if(roleStr2 == "read-only") {
		role = Procedure::Settings::Role::readOnly;
	}
	else if(roleStr2 == "execute") {
		role = Procedure::Settings::Role::execute;
	}
	else if(roleStr2 == "worker") {
		role = Procedure::Settings::Role::worker;
	}
	else {
		throw ArgumentsException("Invalid value \"" + roleStr2 + "\" for role of option \"--group\".");
	}

	namespacesRolesByGroup[group].insert(std::make_pair(namespaceId, role));
}

void Config::addUser(const char* user, const char* group) {
	if(!user) {
		throw ArgumentsException("User name missing of option \"--user\".");
	}

	if(!group) {
		throw ArgumentsException("Group name missing of option \"--user\".");
	}

	groupsByUser[user].insert(group);
}

void Config::addBasicAuth(const char* user, const char* password) {
	if(!user) {
		throw ArgumentsException("User name missing of option \"--basic-auth\".");
	}

	if(!password) {
		throw ArgumentsException("Password missing of option \"--basic-auth\".");
	}

//	groupsByUser[user];
	auto& userData = settings.users[user];
	userData.userName = user;
	if(!userData.pw.empty()) {
		throw ArgumentsException("Cannot specify user \"" + userData.userName + "\" of option \"--basic-auth\" twice.");
	}
	userData.pw = password;
	if(userData.pw.empty()) {
		throw ArgumentsException("Invalid value \"\" for password of option \"--basic-auth\".");
	}
}

void Config::addDatabase(const char* implementation) {
	if(!implementation) {
		throw ArgumentsException("Plugin-value missing of option \"--database\".");
	}

	setSettingState(SettingsState::database);
	database->implementation = implementation;
}

void Config::addObserver(const char* implementation) {
	if(!implementation) {
		throw ArgumentsException("Plugin-value missing of option \"--observer\".");
	}

	setSettingState(SettingsState::observer);
	observer.implementation = implementation;
}

void Config::addSocket(const char* implementation) {
	if(!implementation) {
		throw ArgumentsException("Plugin-value missing of option \"--socket\".");
	}

	setSettingState(SettingsState::socket);
	socket.implementation = implementation;
}

void Config::addConfigFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--config-file\".");
	}
	configFiles.push_back(value);
}

} /* namespace args */
} /* namespace config */
} /* namespace head */
} /* namespace batchelor */
