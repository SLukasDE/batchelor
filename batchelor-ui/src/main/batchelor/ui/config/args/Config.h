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

#ifndef BATCHELOR_UI_CONFIG_ARGS_CONFIG_H_
#define BATCHELOR_UI_CONFIG_ARGS_CONFIG_H_

#include <batchelor/ui/Procedure.h>

#include <esl/object/Context.h>

#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace ui {
namespace config {
namespace args {

class Config {
public:
	static void printUsage();

	Config(esl::object::Context& context, Procedure::Settings& settings, int argc, const char* argv[]);

	const std::vector<std::string>& getConfigFiles() const noexcept;

private:
	esl::object::Context& context;
	Procedure::Settings& settings;

	struct Connection {
		std::string plugin;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Connection connection;
	unsigned int connectionCount = 0;

	struct Socket {
		std::string implementation;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Socket socket;
	unsigned int socketCount = 0;

	enum class SettingsState {
		none, socket, connection
	};
	SettingsState settingState = SettingsState::none;

	std::vector<std::string> configFiles;

	void setSettingState(SettingsState settingState);

	void addConfigFile(const char* value);
	void addSetting(const char* key, const char* value);
	void addCertificate(const char* hostName, const char* keyFile, const char* certFile);
	void addApiKey(const char* user, const char* apik);
	void addBasicAuth(const char* user, const char* password);
	void addUser(const char* user, const char* namespaceId, const char* role);
	void addSocket(const char* implementation);
	void addConnection(const char* plugin);
};

} /* namespace args */
} /* namespace config */
} /* namespace ui */
} /* namespace batchelor */

#endif /* BATCHELOR_UI_CONFIG_ARGS_CONFIG_H_ */
