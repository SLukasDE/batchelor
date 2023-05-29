#include <batchelor/head/Dao.h>
#include <batchelor/head/Logger.h>

#include "sergut/JsonDeserializer.h"
#include "sergut/JsonSerializer.h"

#define ONLY_C_LOCALE 1
#include <date/date.h>

#include <esl/database/PreparedStatement.h>
#include <esl/system/Stacktrace.h>

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
    if(std::is_signed<IType>::value && std::is_signed<OType>::value && inputValue < std::numeric_limits<OType>::min()) {
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

}

Dao::Dao(esl::database::Connection& aDbConnection)
: dbConnection(aDbConnection)
{
	dbConnection.prepare(
			"CREATE TABLE IF NOT EXISTS JOBS("
            "JOB_ID TEXT, "
            "BATCH_ID TEXT, "
            "PRIORITY INTEGER, "
            "ARGUMENTS TEXT, "
            "ENV_VARS BLOB, "
            "SETTINGS BLOB, "
            "CREATED_TS TEXT, "
            "BEGIN_TS TEXT, "
            "END_TS TEXT, "
            "LAST_HEARTBEAT_TS TEXT, "
            "STATE TEXT, "
            "RETURN_CODE INTEGER, "
            "MESSAGE TEXT);").execute();
}

void Dao::saveJob(const Job& job) {
	if(insertJob(job)) {
		return;
	}
	if(updateJob(job)) {
		return;
	}
}

bool Dao::insertJob(const Job& job) {
	static const std::string sqlStr = "INSERT INTO JOBS ("
			"JOB_ID, "
			"BATCH_ID, "
			"PRIORITY, "
			"ARGUMENTS, "
			"ENV_VARS, "
			"SETTINGS, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"STATE, "
			"RETURN_CODE, "
			"MESSAGE TEXT) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	logger.trace << "Dao::insertJob statement: " << sqlStr << "\n";

    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    statement.execute(
        job.jobId,
		job.batchId,
		checkedNumericConvert<int>(job.priority),
		toString(job.arguments),
		toString(job.envVars),
		toString(job.settings),
		toString(job.createdTS),
		toString(job.startTS),
		toString(job.endTS),
		toString(job.lastHeartbeatTS),
		common::types::State::toString(job.state),
		job.returnCode,
		job.message
		);

    return true;
}

bool Dao::updateJob(const Job& job) {
	static const std::string sqlStr = "UPDATE JOBS SET ("
			"BATCH_ID = ?, "
			"PRIORITY = ?, "
			"ARGUMENTS = ?, "
			"ENV_VARS = ?, "
			"SETTINGS = ?, "
			"CREATED_TS = ?, "
			"BEGIN_TS = ?, "
			"END_TS = ?, "
			"LAST_HEARTBEAT_TS = ?, "
			"STATE = ?, "
			"RETURN_CODE = ?, "
			"MESSAGE TEXT = ?) "
			"WHERE JOB_ID = ?;";

	logger.trace << "Dao::insertJob statement: " << sqlStr << "\n";

    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    statement.execute(
		job.batchId,
		checkedNumericConvert<int>(job.priority),
		toString(job.arguments),
		toString(job.envVars),
		toString(job.settings),
		toString(job.createdTS),
		toString(job.startTS),
		toString(job.endTS),
		toString(job.lastHeartbeatTS),
		common::types::State::toString(job.state),
		job.returnCode,
		job.message,
        job.jobId
		);

    return true;
}

std::vector<Dao::Job> Dao::loadJobs(const common::types::State::Type& state) {
    std::vector<Job> results;

	static const std::string sqlStr = "SELECT "
			"JOB_ID, "
			"BATCH_ID, "
			"PRIORITY, "
			"ARGUMENTS, "
			"ENV_VARS, "
			"SETTINGS, "
			"CREATED_TS, "
			"BEGIN_TS, "
			"END_TS, "
			"LAST_HEARTBEAT_TS, "
			"RETURN_CODE, "
			"MESSAGE TEXT "
			"FROM JOBS "
			"WHERE STATE = ?;";
    esl::database::PreparedStatement statement = dbConnection.prepare(sqlStr);

    for(esl::database::ResultSet resultSet = statement.execute(common::types::State::toString(state)); resultSet; resultSet.next()) {
    	Job job;

    	job.jobId = resultSet[0].isNull() ? "" : resultSet[0].asString();
    	job.batchId = resultSet[1].isNull() ? "" : resultSet[1].asString();
    	job.priority = resultSet[2].isNull() ? 0 : resultSet[2].asInteger();
        job.arguments = toArguments(resultSet[3].isNull() ? "" : resultSet[3].asString());
        job.settings = toSettings(resultSet[4].isNull() ? "" : resultSet[4].asString());
        job.envVars = toSettings(resultSet[5].isNull() ? "" : resultSet[5].asString());
		job.createdTS = toTimepoint(resultSet[6].isNull() ? "" : resultSet[6].asString());
		job.startTS = toTimepoint(resultSet[7].isNull() ? "" : resultSet[7].asString());
		job.endTS = toTimepoint(resultSet[8].isNull() ? "" : resultSet[8].asString());
		job.lastHeartbeatTS = toTimepoint(resultSet[9].isNull() ? "" : resultSet[9].asString());
    	job.state = state;
    	job.returnCode = resultSet[10].isNull() ? 0 : resultSet[10].asInteger();
    	job.message = resultSet[11].isNull() ? "" : resultSet[11].asString();

        results.push_back(job);
    }

    logger.debug << "Active StoreWarehouses: " << results.size() << "\n";

    return results;
}

} /* namespace head */
} /* namespace batchelor */
