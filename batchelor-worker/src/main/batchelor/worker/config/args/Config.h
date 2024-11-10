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

#ifndef BATCHELOR_WORKER_CONFIG_ARGS_CONFIG_H_
#define BATCHELOR_WORKER_CONFIG_ARGS_CONFIG_H_

#include <batchelor/worker/Procedure.h>

#include <esl/object/Context.h>

#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
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

	struct Event {
		std::string id;
		std::string type;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Event event;

	struct Connection {
		std::string plugin;
		std::vector<std::pair<std::string, std::string>> settings;
	};
	Connection connection;
	unsigned int connectionCount = 0;

	std::vector<std::string> configFiles;

	enum class SettingsState {
		none, event, connection
	} settingState = SettingsState::none;

	void setSettingState(SettingsState settingState);

	void setNamespaceId(const char* namespaceId);
	void setIdleTimeout(const char* maximumTasksRunning);
	void setAvailableTimeout(const char* maximumTasksRunning);
	void setWorkerId(const char* workerId);
	void addMetrics(const char* key, const char* value);
	void addConfigFile(const char* value);
	void addSettings(const char* key, const char* value);
	void addHttpPort(const char* portStr);

	void addEvent(const char* id, const char* plugin);

	void addConnection(const char* plugin);
};

} /* namespace args */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CONFIG_ARGS_CONFIG_H_ */
