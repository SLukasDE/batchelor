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

#include <batchelor/common/auth/UserData.h>
#include <batchelor/common/config/args/ArgumentsException.h>
#include <batchelor/common/plugin/Socket.h>

#include <batchelor/head/config/args/Config.h>
#include <batchelor/head/plugin/Observer.h>

#include <esl/crypto/KeyStore.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/database/SQLiteConnectionFactory.h>
#include <esl/plugin/Registry.h>
#include <esl/utility/String.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace batchelor {
namespace head {
namespace config {
namespace args {

using batchelor::common::config::args::ArgumentsException;
using esl::utility::String;

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
	std::cout << "  -U, --user             <user> <ns> <role> Defines an user <user> to have a role <role> at namespace <ns>\n";
	std::cout << "                                            To add several role-to-namespace settings to an user, just repeat the definition\n";
	std::cout << "                                            several times with the same user <user>, but different values for <role> and <ns>.\n";
	std::cout << "                                            Following roles are available:\n";
	std::cout << "                                            * read-only: A controller needs this role to watch information of tasks.\n";
	std::cout << "                                            * execute:   A controller needs this role to send events-\n";
	std::cout << "                                            * worker:    A worker needs this role to provide event types.\n";
	std::cout << "\n";
	std::cout << "  -A, --api-key          <user> <api-key>   Defines an API key for a user <user> used if bearer-authentication is used over https.\n";
	std::cout << "                                            The API key is specified in <api-key> that has format \"<encryption>:<value>\".\n";
	std::cout << "                                            There are several values available for <encryption>:\n";
	std::cout << "                                            * plain:<value>      Defines the API key as plain text in <value>.\n";
	std::cout << "\n";
	std::cout << "  -B, --basic-auth       <user> <pw>        Defines a password for a user <user> used if basic-authentication is used over https.\n";
	std::cout << "                                            The password is specified in <pw> that has format \"<encryption>:<value>\".\n";
	std::cout << "                                            There are several values available for <encryption>:\n";
	std::cout << "                                            * plain:<value>      Defines the password as plain text in <value>.\n";
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
	std::cout << "                                            Most popular used plugin is \"basic\" with following settings:\n";
	std::cout << "                                            * port:          <number>      Defines the port number to listen for HTTP requests.\n";
	std::cout << "                                            * threads:       <number>      Defines the number of requests to handle parallel.\n";
	std::cout << "                                            * https:         <true|false>  Defines if http or https is used.\n";
}

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
			addSetting(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-c"  || currentArg == "--certificate") {
			addCertificate(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr, i+3 < argc ? argv[i+3] : nullptr);
			i = i+3;
		}
		else if(currentArg == "-U"  || currentArg == "--user") {
			addUser(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr, i+3 < argc ? argv[i+3] : nullptr);
			i = i+3;
		}
		else if(currentArg == "-A"  || currentArg == "--api-key") {
			addApiKey(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
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

		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::Socket>(socket.implementation, socket.settings));
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
		context.addObject("batchelor-db", esl::database::SQLiteConnectionFactory::createNative(esl::database::SQLiteConnectionFactory::Settings({
			//{{"URI"}, {":memory:"}}
			{{"URI"}, {"file:test?mode=memory"}}
			//{{"URI"}, {"file::memory:?mode=rw"}}
		})));
	}
}

void Config::addSetting(const char* key, const char* value) {
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
	std::vector<unsigned char> key;
	std::vector<unsigned char> certificate;

	if(!hostName) {
		throw ArgumentsException("Host name missing of option \"--certificate\".");
	}

	if(!keyFile) {
		throw ArgumentsException("Key file missing of option \"--certificate\".");
	}

	if(!certFile) {
		throw ArgumentsException("Certificate file missing of option \"--certificate\".");
	}

	if(keyFile[0] != '*' && keyFile[1] != 0) {
		std::ifstream ifStream(keyFile, std::ios::binary );
		if(!ifStream.good()) {
			throw ArgumentsException("Cannot open key file \"" + std::string(keyFile) + "\"");
		}
	    key = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}

	if(certFile[0] != '*' && certFile[1] != 0) {
		std::ifstream ifStream(certFile, std::ios::binary );
		if(!ifStream.good()) {
			throw ArgumentsException("Cannot open certificate file \"" + std::string(certFile) + "\"");
		}
		certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}



	esl::crypto::KeyStore* keyStore = esl::plugin::Registry::get().findObject<esl::crypto::KeyStore>();
	if(!keyStore) {
		throw ArgumentsException("Cannot add key and certificate, because there is no crypto engine installed.");
	}

	keyStore->addCertificate(hostName, certificate);
	keyStore->addPrivateKey(hostName, key, "");
}

void Config::addApiKey(const char* user, const char* apikey) {
	if(!user) {
		throw ArgumentsException("User name missing of option \"--basic-auth\".");
	}

	if(!apikey) {
		throw ArgumentsException("API key missing of option \"--api-key\".");
	}

	std::string apik = apikey;
	if(apik.substr(0, 6) == "plain:") {
		settings.userByPlainApiKey[apik.substr(6)] = user;
	}
	else {
	//if(userData.pw.empty()) {
		throw ArgumentsException("Invalid value \"" + apik + "\" for api-key of option \"--api-key\".");
	}
}

void Config::addBasicAuth(const char* user, const char* password) {
	if(!user) {
		throw ArgumentsException("User name missing of option \"--basic-auth\".");
	}

	if(!password) {
		throw ArgumentsException("Password missing of option \"--basic-auth\".");
	}

	std::string pw = password;
	if(pw.substr(0, 6) == "plain:") {
		settings.plainBasicAuthByUser[user] = pw.substr(6);
	}

	if(settings.plainBasicAuthByUser[user].empty()) {
		throw ArgumentsException("Invalid value \"" + pw + "\" for password of option \"--basic-auth\".");
	}
}

void Config::addUser(const char* user, const char* namespaceId, const char* roleStr) {
	if(!user) {
		throw ArgumentsException("User name missing of option \"--user\".");
	}

	if(!namespaceId) {
		throw ArgumentsException("Namespace missing of option \"--user\".");
	}

	if(!roleStr) {
		throw ArgumentsException("Role missing of option \"--user\".");
	}

	auto& userData = settings.users[user];
	userData.rolesByNamespace[namespaceId].insert(common::auth::UserData::toRole(roleStr));
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
