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

#ifndef BATCHELOR_CONTROL_CONFIG_ARGS_CONFIG_H_
#define BATCHELOR_CONTROL_CONFIG_ARGS_CONFIG_H_

#include <batchelor/control/Command.h>
#include <batchelor/control/Procedure.h>

#include <esl/object/Context.h>

#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace control {
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

	std::vector<std::string> configFiles;

	enum class SettingsState {
		none, event, connection
	} settingState = SettingsState::none;

	void setSettingState(SettingsState settingState);

	void setCommand(const std::string& commandStr);
	const Command& getCommand() const;

	void setNamespaceId(const char* namespaceId);
	void setEventType(const char* eventType);
	void setPriority(const char* priority);
	void addMetric(const char* key, const char* value);
	void addSetting(const char* key, const char* value);
	void setCondition(const char* condition);
	void setWait();
	void setWaitCancel(const char* maxTries);
	void setTaskId(const char* taskId);
	void setSignal(const char* signalName);
	void setState(const char* state);
	void setEventNotAfter(const char* eventNotAfter);
	void setEventNotBefore(const char* eventNotBefore);

	void addConnection(const char* plugin);
	void addConnectionFile(const char* value);
};

} /* namespace args */
} /* namespace config */
} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_CONFIG_ARGS_CONFIG_H_ */
