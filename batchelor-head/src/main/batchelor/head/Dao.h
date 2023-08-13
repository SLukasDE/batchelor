#ifndef BATCHELOR_HEAD_DAO_H_
#define BATCHELOR_HEAD_DAO_H_

#include <batchelor/service/schemas/Setting.h>
#include <batchelor/common/types/State.h>

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
#if 0
	enum class State {
		queued
	};
#endif
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

	void saveTask(const Task& task);
	bool insertTask(const Task& task);
	bool updateTask(const Task& task);

	std::vector<Task> loadTasks(const common::types::State::Type& state, const std::chrono::system_clock::time_point& eventNotAfter, const std::chrono::system_clock::time_point& eventNotBefore);
	std::unique_ptr<Task> loadTaskByTaskId(const std::string& taskId);
	std::unique_ptr<Task> loadLatesTaskByEventTypeAndCrc32(const std::string& eventType, std::uint32_t crc32);
	std::vector<Task> loadTasksByEventTypeAndState(const std::string& eventType, const common::types::State::Type& state);

	esl::database::Connection& dbConnection;
	const bool isSQLite;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_DAO_H_ */
