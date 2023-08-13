#include <batchelor/head/Dao.h>
#include <batchelor/head/Logger.h>

#include "sergut/JsonDeserializer.h"
#include "sergut/JsonSerializer.h"

#define ONLY_C_LOCALE 1
#include <date/date.h>

#include <esl/database/PreparedStatement.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <ctime>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <time.h>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Dao");


template<typename OType, typename IType>
OType checkedNumericConvert(IType inputValue) {
    if(std::is_signed<IType>::value && std::is_unsigned<OType>::value && inputValue < 0) {
        throw esl::system::Stacktrace::add(std::runtime_error("Converting negative number to unsigned integer type"));
    }
    if(std::is_signed<IType>::value && std::is_signed<OType>::value && inputValue < std::numeric_limits<OType>::lowest()) {
        throw esl::system::Stacktrace::add(std::runtime_error("Numeric underflow in conversion"));
    }
    if(inputValue > std::numeric_limits<OType>::max()) {
        throw esl::system::Stacktrace::add(std::runtime_error("Numeric overflow in conversion"));
    }
    return static_cast<OType>(inputValue);
}

// ---------------------------------------------------------------------------------------------

std::string toString(const std::chrono::system_clock::time_point& tp) {
	return date::format("%F %T", tp);
}

std::chrono::system_clock::time_point toTimepoint(const std::string& tpStr) {
	std::chrono::system_clock::time_point tp;

	if(!tpStr.empty()) {
		std::istringstream ss(tpStr);
		date::from_stream(ss, "%F %T", tp);
	}

	return tp;
}

// ---------------------------------------------------------------------------------------------

std::string toString(const std::vector<std::string>& arguments) {
	sergut::JsonSerializer serializer;
	serializer.serializeData(arguments);
	return serializer.str();
}

std::vector<std::string> toArguments(const std::string& arguments) {
	if(!arguments.empty()) {
	    sergut::JsonDeserializer deSerializer(arguments);
	    return deSerializer.deserializeData<std::vector<std::string>>();
	}
	return {};
}

// ---------------------------------------------------------------------------------------------

std::string toString(const std::vector<service::schemas::Setting>& settings) {
	sergut::JsonSerializer serializer;
	serializer.serializeData(settings);
	return serializer.str();
}

std::vector<service::schemas::Setting> toSettings(const std::string& settings) {
	if(!settings.empty()) {
	    sergut::JsonDeserializer deSerializer(settings);
	    return deSerializer.deserializeData<std::vector<service::schemas::Setting>>();
	}
	return {};
}

unsigned int calculatedEffectivePriority(unsigned int priority, std::chrono::system_clock::time_point priorityTS) {
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now()-priorityTS).count();
	if(minutes >= 24) {
		minutes = 24;
		logger.info << "Minutes = " << minutes << "\n";
	}
	if(minutes < 0) {
		logger.warn << "Minutes = " << minutes << "\n";
		minutes = 0;
	}

	return priority + minutes;
}
}

Dao::Dao(esl::database::Connection& aDbConnection)
: dbConnection(aDbConnection),
  isSQLite(dbConnection.getImplementations().count("SQLite") > 0)
{
	if(isSQLite) {
		dbConnection.prepare(
			"CREATE TABLE IF NOT EXISTS TASKS("
            "TASK_ID TEXT, "
            "CRC32 INTEGER, "
            "PRIORITY INTEGER, "
            "PRIORITY_TS INTEGER, "
	        "EVENT_TYPE TEXT, "
	        "SETTINGS BLOB, "
	        "METRICS BLOB, "
            "SIGNALS TEXT, "
            "CONDITION TEXT, "
            "CREATED_TS INTEGER, "
            "BEGIN_TS TEXT, "
            "END_TS TEXT, "
            "LAST_HEARTBEAT_TS INTEGER, "
            "STATE TEXT, "
            "RETURN_CODE INTEGER, "
            "MESSAGE TEXT);").execute();
	}
}

void Dao::saveTask(const Task& task) {
	if(insertTask(task)) {
		return;
	}
	if(updateTask(task)) {
		return;
	}
}

bool Dao::insertTask(const Task& task) {
	static const std::string sqlStr = "INSERT INTO TASKS ("
			"TASK_ID, "
			"CRC32, "
			"PRIORITY, "
			"PRIORITY_TS, "
	        "EVENT_TYPE, "
			"SETTINGS, "
			"METRICS, "
			"SIGNALS, "
            "CONDITION, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"STATE, "
			"RETURN_CODE, "
			"MESSAGE) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, '', ?, ?, ?, ?, ?, ?, ?, ?);";

	logger.trace << "Dao::insertTask statement: " << sqlStr << "\n";

    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    std::int64_t createdTSDuration = std::chrono::time_point_cast<std::chrono::milliseconds>(task.createdTS).time_since_epoch().count();

    statement.execute(
		task.taskId,
		checkedNumericConvert<std::int64_t>(task.crc32),
		checkedNumericConvert<int>(task.priority),
		createdTSDuration,
		task.eventType,
		toString(task.settings),
		toString(task.metrics),
		task.condition,
		createdTSDuration,
		toString(task.startTS),
		toString(task.endTS),
		createdTSDuration,
		common::types::State::toString(task.state),
		task.returnCode,
		task.message
		);

    return true;
}

bool Dao::updateTask(const Task& task) {
	static const std::string sqlStr = "UPDATE TASKS SET "
			"CRC32 = ?, "
			"PRIORITY = ?, "
			"PRIORITY_TS = ?, "
			"SETTINGS = ?, "
			"METRICS = ?, "
			"SIGNALS = ?, "
			"CONDITION = ?, "
			"CREATED_TS = ?, "
			"BEGIN_TS = ?, "
			"END_TS = ?, "
			"LAST_HEARTBEAT_TS = ?, "
			"STATE = ?, "
			"RETURN_CODE = ?, "
			"MESSAGE = ? "
			"WHERE TASK_ID = ?;";

	logger.trace << "Dao::insertTask statement: " << sqlStr << "\n";

    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    std::int64_t priorityTSDuration = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    std::int64_t lastCreatedTSDuration = std::chrono::time_point_cast<std::chrono::milliseconds>(task.createdTS).time_since_epoch().count();
    std::int64_t lastHeartbeatTSDuration = std::chrono::time_point_cast<std::chrono::milliseconds>(task.lastHeartbeatTS).time_since_epoch().count();
    std::string signals;
    for(const auto& signal : task.signals) {
    	if(!signals.empty()) {
    		signals += ",";
    	}
    	signals += signal;
    }

    statement.execute(
   		checkedNumericConvert<std::int64_t>(task.crc32),
		checkedNumericConvert<int>(task.priority),
		priorityTSDuration,
		toString(task.settings),
		toString(task.metrics),
		signals,
		task.condition,
		lastCreatedTSDuration,
		toString(task.startTS),
		toString(task.endTS),
		lastHeartbeatTSDuration,
		common::types::State::toString(task.state),
		task.returnCode,
		task.message,
		task.taskId
		);

    return true;
}

std::vector<Dao::Task> Dao::loadTasks(const common::types::State::Type& state, const std::chrono::system_clock::time_point& eventNotAfterTS, const std::chrono::system_clock::time_point& eventNotBeforeTS) {
    std::vector<Task> results;

	static const std::string sqlStr = "SELECT "
			"TASK_ID, "
			"CRC32, "
			"PRIORITY, "
			"PRIORITY_TS, "
			"EVENT_TYPE, "
			"SETTINGS, "
			"METRICS, "
			"SIGNALS, "
			"CONDITION, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"RETURN_CODE, "
			"MESSAGE "
			"FROM TASKS "
			"WHERE STATE = ? AND CREATED_TS >= ? AND CREATED_TS <= ?;";
    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);
    std::int64_t eventNotAfter = std::chrono::time_point_cast<std::chrono::milliseconds>(eventNotAfterTS).time_since_epoch().count();
    std::int64_t eventNotBefore = std::chrono::time_point_cast<std::chrono::milliseconds>(eventNotBeforeTS).time_since_epoch().count();

    for(esl::database::ResultSet resultSet = statement.execute(common::types::State::toString(state), eventNotBefore, eventNotAfter); resultSet; resultSet.next()) {
    	Task task;

    	task.taskId = resultSet[0].isNull() ? "" : resultSet[0].asString();
    	task.crc32 = resultSet[1].isNull() ? 0 : resultSet[1].asInteger();
    	task.priority = resultSet[2].isNull() ? 0 : resultSet[2].asInteger();
    	if(!resultSet[3].isNull()) {
    	    task.priorityTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[3].asInteger()));
    	}
    	task.effectivePriority = calculatedEffectivePriority(task.priority, task.priorityTS);
    	task.eventType = resultSet[4].isNull() ? "" : resultSet[4].asString();
    	task.settings = toSettings(resultSet[5].isNull() ? "" : resultSet[5].asString());
    	task.metrics = toSettings(resultSet[6].isNull() ? "" : resultSet[6].asString());
    	if(!resultSet[7].isNull()) {
    		task.signals = esl::utility::String::split(resultSet[7].asString(), ',', true);
    	}
    	task.condition = resultSet[8].isNull() ? "" : resultSet[8].asString();
    	if(!resultSet[9].isNull()) {
    	    task.createdTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[9].asInteger()));
    	}
    	task.startTS = toTimepoint(resultSet[10].isNull() ? "" : resultSet[10].asString());
    	task.endTS = toTimepoint(resultSet[11].isNull() ? "" : resultSet[11].asString());
    	if(!resultSet[12].isNull()) {
    	    task.lastHeartbeatTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[12].asInteger()));
    	}
    	task.state = state;
    	task.returnCode = resultSet[13].isNull() ? 0 : resultSet[13].asInteger();
    	task.message = resultSet[14].isNull() ? "" : resultSet[14].asString();

        results.push_back(task);
    }

    logger.debug << "Active StoreWarehouses: " << results.size() << "\n";

    return results;
}

std::unique_ptr<Dao::Task> Dao::loadTaskByTaskId(const std::string& taskId) {
    std::unique_ptr<Task> task;

	static const std::string sqlStr = "SELECT "
			"CRC32, "
			"PRIORITY, "
			"PRIORITY_TS, "
			"EVENT_TYPE, "
			"SETTINGS, "
			"METRICS, "
			"SIGNALS, "
			"CONDITION, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"STATE, "
			"RETURN_CODE, "
			"MESSAGE "
			"FROM TASKS "
			"WHERE TASK_ID = ?;";
    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    esl::database::ResultSet resultSet = statement.execute(taskId);

    if(resultSet) {
    	task.reset(new Task);

    	task->taskId = taskId;
    	task->crc32 = resultSet[0].isNull() ? 0 : resultSet[0].asInteger();
    	task->priority = resultSet[1].isNull() ? 0 : resultSet[1].asInteger();
    	if(!resultSet[2].isNull()) {
    	    task->priorityTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[2].asInteger()));
    	}
    	task->effectivePriority = calculatedEffectivePriority(task->priority, task->priorityTS);
    	task->eventType = resultSet[3].isNull() ? "" : resultSet[3].asString();
    	task->settings = toSettings(resultSet[4].isNull() ? "" : resultSet[4].asString());
    	task->metrics = toSettings(resultSet[5].isNull() ? "" : resultSet[5].asString());
    	if(!resultSet[6].isNull()) {
    		task->signals = esl::utility::String::split(resultSet[6].asString(), ',', true);
    	}
    	task->condition = resultSet[7].isNull() ? "" : resultSet[7].asString();
    	if(!resultSet[8].isNull()) {
    		task->createdTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[8].asInteger()));
    	}
    	task->startTS = toTimepoint(resultSet[9].isNull() ? "" : resultSet[9].asString());
    	task->endTS = toTimepoint(resultSet[10].isNull() ? "" : resultSet[10].asString());
    	if(!resultSet[11].isNull()) {
    	    task->lastHeartbeatTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[11].asInteger()));
    	}
    	task->state = resultSet[12].isNull() ? common::types::State::zombie : common::types::State::toState(resultSet[12].asString());
    	task->returnCode = resultSet[13].isNull() ? 0 : resultSet[13].asInteger();
    	task->message = resultSet[14].isNull() ? "" : resultSet[14].asString();
    }
	return task;
}

std::unique_ptr<Dao::Task> Dao::loadLatesTaskByEventTypeAndCrc32(const std::string& eventType, std::uint32_t crc32) {
    std::unique_ptr<Task> task;

	static const std::string sqlStr = "SELECT "
			"TASK_ID, "
			"STATE, "
			"PRIORITY, "
			"PRIORITY_TS, "
			"SETTINGS, "
			"METRICS, "
			"SIGNALS, "
			"CONDITION, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"RETURN_CODE, "
			"MESSAGE "
			"FROM TASKS "
			"WHERE EVENT_TYPE = ? AND CRC32 = ?;";
    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    for(esl::database::ResultSet resultSet = statement.execute(eventType, checkedNumericConvert<std::int64_t>(crc32)); resultSet; resultSet.next()) {
    	std::chrono::system_clock::time_point createdTS;
    	if(!resultSet[8].isNull()) {
    		createdTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[8].asInteger()));
    	}

		if(task && task->createdTS > createdTS) {
			continue;
		}

    	task.reset(new Task);
    	task->eventType = eventType;
    	task->crc32 = crc32;

    	task->taskId = resultSet[0].isNull() ? "" : resultSet[0].asString();
    	task->state = resultSet[1].isNull() ? common::types::State::zombie : common::types::State::toState(resultSet[1].asString());
    	task->priority = resultSet[2].isNull() ? 0 : resultSet[2].asInteger();
    	if(!resultSet[3].isNull()) {
    	    task->priorityTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[3].asInteger()));
    	}
    	task->effectivePriority = calculatedEffectivePriority(task->priority, task->priorityTS);
    	task->settings = toSettings(resultSet[4].isNull() ? "" : resultSet[4].asString());
    	task->metrics = toSettings(resultSet[5].isNull() ? "" : resultSet[5].asString());
    	if(!resultSet[6].isNull()) {
    		task->signals = esl::utility::String::split(resultSet[6].asString(), ',', true);
    	}
    	task->condition = resultSet[7].isNull() ? "" : resultSet[7].asString();
	    task->createdTS = createdTS;
    	task->startTS = toTimepoint(resultSet[9].isNull() ? "" : resultSet[9].asString());
    	task->endTS = toTimepoint(resultSet[10].isNull() ? "" : resultSet[10].asString());
    	if(!resultSet[11].isNull()) {
    	    task->lastHeartbeatTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[11].asInteger()));
    	}
    	task->returnCode = resultSet[12].isNull() ? 0 : resultSet[12].asInteger();
    	task->message = resultSet[13].isNull() ? "" : resultSet[13].asString();
    }
	return task;
}

std::vector<Dao::Task> Dao::loadTasksByEventTypeAndState(const std::string& eventType, const common::types::State::Type& state) {
    std::vector<Task> tasks;

	static const std::string sqlStr = "SELECT "
			"CRC32, "
			"TASK_ID, "
			"PRIORITY, "
			"PRIORITY_TS, "
			"SETTINGS, "
			"METRICS, "
			"SIGNALS, "
			"CONDITION, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"RETURN_CODE, "
			"MESSAGE "
			"FROM TASKS "
			"WHERE EVENT_TYPE = ? AND STATE = ?;";
    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    for(esl::database::ResultSet resultSet = statement.execute(eventType, common::types::State::toString(common::types::State::queued)); resultSet; resultSet.next()) {
    	Task task;

    	task.eventType = eventType;
    	task.state = common::types::State::queued;
    	task.crc32 = resultSet[0].isNull() ? 0 : resultSet[0].asInteger();
    	task.taskId = resultSet[1].isNull() ? "" : resultSet[1].asString();
    	task.priority = resultSet[2].isNull() ? 0 : resultSet[2].asInteger();
    	if(!resultSet[3].isNull()) {
    	    task.priorityTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[3].asInteger()));
    	}
    	task.effectivePriority = calculatedEffectivePriority(task.priority, task.priorityTS);

    	task.settings = toSettings(resultSet[4].isNull() ? "" : resultSet[4].asString());
    	task.metrics = toSettings(resultSet[5].isNull() ? "" : resultSet[5].asString());
    	if(!resultSet[6].isNull()) {
    		task.signals = esl::utility::String::split(resultSet[6].asString(), ',', true);
    	}
    	task.condition = resultSet[7].isNull() ? "" : resultSet[7].asString();
    	if(!resultSet[8].isNull()) {
    		task.createdTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[8].asInteger()));
    	}
    	task.startTS = toTimepoint(resultSet[9].isNull() ? "" : resultSet[9].asString());
    	task.endTS = toTimepoint(resultSet[10].isNull() ? "" : resultSet[10].asString());
    	if(!resultSet[11].isNull()) {
    	    task.lastHeartbeatTS = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(resultSet[11].asInteger()));
    	}
    	task.returnCode = resultSet[12].isNull() ? 0 : resultSet[12].asInteger();
    	task.message = resultSet[13].isNull() ? "" : resultSet[13].asString();

    	tasks.push_back(task);
    }

    return tasks;
}

} /* namespace head */
} /* namespace batchelor */
