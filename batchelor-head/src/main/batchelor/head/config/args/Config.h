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

#ifndef BATCHELOR_HEAD_CONFIG_ARGS_CONFIG_H_
#define BATCHELOR_HEAD_CONFIG_ARGS_CONFIG_H_

#include <batchelor/head/Procedure.h>

#include <esl/object/Context.h>

#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
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

	struct Database {
		std::string implementation;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	std::unique_ptr<Database> database;

	struct Observer {
		std::string implementation;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Observer observer;
	unsigned int observerCount = 0;

	struct Socket {
		std::string implementation;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Socket socket;
	unsigned int socketCount = 0;

	enum class SettingsState {
		none, database, observer, socket
	} settingState = SettingsState::none;

	void setSettingState(SettingsState settingState);

	void addSetting(const char* key, const char* value);
	void addCertificate(const char* hostName, const char* keyFile, const char* certFile);
	void addApiKey(const char* user, const char* apik);
	void addBasicAuth(const char* user, const char* password);
	void addUser(const char* user, const char* namespaceId, const char* role);
	void addDatabase(const char* implementation);
	void addObserver(const char* implementation);
	void addSocket(const char* implementation);

	std::vector<std::string> configFiles;

	void addConfigFile(const char* value);
};

} /* namespace args */
} /* namespace config */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CONFIG_ARGS_CONFIG_H_ */
