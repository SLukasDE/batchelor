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
#include <batchelor/common/plugin/Socket.h>
#include <batchelor/common/types/State.h>

#include <batchelor/ui/config/args/Config.h>

#include <esl/crypto/KeyStore.h>
#include <esl/plugin/Registry.h>
#include <esl/utility/String.h>

#include <fstream>
#include <iostream>

namespace batchelor {
namespace ui {
namespace config {
namespace args {

using batchelor::common::config::args::ArgumentsException;

namespace {
ArgumentsException argumentsExceptionCommandOptionMismatch(const std::string& commandStr, const std::string& optionStr) {
	return ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '" + optionStr + "'.");
}

} /* anonymous namespace */

void Config::printUsage() {
	std::cout << "batchelor-ui [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor-ui help\n";
	std::cout << "  batchelor-ui -C basic [CONNECTION SETTINGS] -S basic [SOCKET SETTINGS] [--config-file <file>]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -f, --config-file      <file>             Configuration file\n";
	std::cout << "\n";
	std::cout << "  -s, --setting          <key> <value>      Observer or socket specific setting.\n";
	std::cout << "\n";
	std::cout << "  -c, --certificate      <host> <key> <crt> Defines a certificate to use for hostname <host>. The certificate is repesented as\n";
	std::cout << "                                            two files, the key-file, specified by <key> and the cert-file, specified by <crt>.\n";
	std::cout << "\n";
	std::cout << "  -S, --socket           <plugin>           Defines a socket to listen for requests.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "                                            Most popular used plugin is \"basic\" with following settings:\n";
	std::cout << "                                            * port:          <number>      Defines the port number to listen for HTTP requests.\n";
	std::cout << "                                            * threads:       <number>      Defines the number of requests to handle parallel.\n";
	std::cout << "                                            * https:         <true|false>  Defines if http or https is used.\n";
	std::cout << "\n";
	std::cout << "  -C, --connection       <plugin>           Defines the connection to a head server.\n";
	std::cout << "                                            Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "                                            Most popular used plugin is \"basic\" with following settings:\n";
	std::cout << "                                            * url:           <server-url>  Defines the URL to the head server.\n";
	std::cout << "                                            * username:      <username>    If this setting is specified, basic-auth will be used.\n";
	std::cout << "                                            * password:      <password>    If this setting is specified, basic-auth will be used.\n";
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
		else if(currentArg == "-S"  || currentArg == "--socket") {
			addSocket(i+1 < argc ? argv[i+1] : nullptr);
			++i;
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
		std::string id = "batchelor-connection-" + std::to_string(connectionCount);

		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::ConnectionFactory>(connection.plugin, connection.settings));
		if(settings.connectionFactoryIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of server connection with <id> = \"" + id + "\".");
		}
	}

	else if(settingState == SettingsState::socket) {
		++socketCount;
		std::string id = "batchelor-socket-" + std::to_string(socketCount);

		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::Socket>(socket.implementation, socket.settings));
		if(settings.socketIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of socket with <id> = \"" + id + "\".");
		}
	}

	settingState = aSettingState;

	connection = Connection();
	socket = Socket();
}

void Config::addConfigFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--config-file\".");
	}
	configFiles.push_back(value);
}

void Config::addSetting(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--setting\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--setting\".");
	}

	switch(settingState) {
	case SettingsState::none:
		throw ArgumentsException("Option \"--setting\" is invalid, because there is no previous option \"--connection\" or \"--socket\".");
	case SettingsState::socket:
		socket.settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
		break;
	case SettingsState::connection:
		connection.settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
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

void Config::addSocket(const char* implementation) {
	if(!implementation) {
		throw ArgumentsException("Plugin-value missing of option \"--socket\".");
	}

	setSettingState(SettingsState::socket);
	socket.implementation = implementation;
}

void Config::addConnection(const char* plugin) {
	if(!plugin) {
		throw ArgumentsException("Plugin-value missing of option \"--connection\".");
	}

	setSettingState(SettingsState::connection);
	connection.plugin = plugin;
}

} /* namespace args */
} /* namespace config */
} /* namespace ui */
} /* namespace batchelor */
