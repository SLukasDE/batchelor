#ifndef BATCHELOR_HEAD_DAO_H_
#define BATCHELOR_HEAD_DAO_H_

#include <batchelor/service/schemas/Setting.h>
#include <batchelor/common/types/State.h>

#include <esl/database/Connection.h>

#include <chrono>
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
	struct Job {
		std::string jobId;
		std::string batchId;
		unsigned int priority = 0;
		std::vector<std::string> arguments;
		std::vector<service::schemas::Setting> envVars;
		std::vector<service::schemas::Setting> settings;

		std::chrono::system_clock::time_point createdTS;
		std::chrono::system_clock::time_point startTS;
		std::chrono::system_clock::time_point endTS;
		std::chrono::system_clock::time_point lastHeartbeatTS;

		common::types::State::Type state;
		int returnCode = 0;
		std::string message;
	};

	Dao(esl::database::Connection& dbConnection);

	void saveJob(const Job& job);
	std::vector<Job> loadJobs(const common::types::State::Type& state);

private:
	bool insertJob(const Job& job);
	bool updateJob(const Job& job);

	esl::database::Connection& dbConnection;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_DAO_H_ */
