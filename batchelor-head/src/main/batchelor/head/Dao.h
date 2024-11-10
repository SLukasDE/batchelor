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

#ifndef BATCHELOR_HEAD_DAO_H_
#define BATCHELOR_HEAD_DAO_H_

#include <batchelor/common/types/State.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/database/Connection.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace head {

class Dao {
public:
	struct Task {
		std::string taskId;
		std::uint32_t crc32;
		std::string eventType;
		unsigned int priority = 0;
		std::chrono::system_clock::time_point priorityTS;
		unsigned int effectivePriority = 0;
		std::vector<service::schemas::Setting> settings;
		std::vector<service::schemas::Setting> metrics;
		std::vector<std::string> signals;
		std::string condition;

		std::chrono::system_clock::time_point createdTS;
		std::chrono::system_clock::time_point startTS;
		std::chrono::system_clock::time_point endTS;
		std::chrono::system_clock::time_point lastHeartbeatTS;

		common::types::State::Type state;
		int returnCode = 0;
		std::string message;
	};

	Dao(esl::database::Connection& dbConnection);

	void saveTask(const std::string& namespaceId, const Task& task);
	bool insertTask(const std::string& namespaceId, const Task& task);
	bool updateTask(const std::string& namespaceId, const Task& task);

	std::vector<Task> loadTasks(const std::string& namespaceId, const std::string& state, const std::chrono::system_clock::time_point& eventNotAfter, const std::chrono::system_clock::time_point& eventNotBefore);
	std::unique_ptr<Task> loadTaskByTaskId(const std::string& namespaceId, const std::string& taskId);
	std::unique_ptr<Task> loadLatesTaskByEventTypeAndCrc32(const std::string& namespaceId, const std::string& eventType, std::uint32_t crc32);
	std::vector<Task> loadTasksByEventTypeAndState(const std::string& namespaceId, const std::string& eventType, const batchelor::common::types::State::Type& state);

	// insert or update given event types with current timestamp
	void updateEventTypes(const std::vector<std::pair<std::string, std::string>>& eventTypes);

	// load all event types, delete outdated event types and return remaining event types
	std::vector<std::string> loadEventTypes(const std::string& namespaceId);

	void cleanup(std::chrono::milliseconds timeoutZombie, std::chrono::milliseconds timeoutCleanup);

private:

	esl::database::Connection& dbConnection;
	const bool isSQLite;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_DAO_H_ */
