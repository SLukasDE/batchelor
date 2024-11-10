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

#ifndef BATCHELOR_WORKER_PROCEDURE_H_
#define BATCHELOR_WORKER_PROCEDURE_H_

#include <batchelor/common/Procedure.h>
#include <batchelor/common/plugin/ConnectionFactory.h>

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/Service.h>

#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/server/Socket.h>
#include <esl/object/Context.h>
#include <esl/object/Procedure.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class Procedure : public common::Procedure {
public:
	struct Settings {
		Settings();
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);

		std::string namespaceId;
		std::string workerId;

		std::vector<std::pair<std::string, std::string>> metrics;
		std::chrono::milliseconds requestInterval{5000};
		std::chrono::milliseconds idleTimeout{0};
		std::chrono::milliseconds availableTimeout{0};
		std::set<std::string> taskFactoryIds;
		std::set<std::string> connectionFactoryIds;
		std::uint16_t alivePort = 0;
	};

	static std::unique_ptr<esl::object::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Procedure(const Settings& settings);
	~Procedure();

	void procedureCancel() override;

	void initializeContext(esl::object::Context& context) override;

protected:
	void internalProcedureRun(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		std::map<std::string, std::reference_wrapper<plugin::TaskFactory>> taskFactroyByEventType;
		std::vector<std::pair<std::string, std::reference_wrapper<common::plugin::ConnectionFactory>>> connectionFactories;
		std::map<std::string, int> resourcesAvailable;
	};

	std::map<std::string, int> getResourcesAvailable() const;
	std::vector<std::pair<std::string, std::string>> getCurrentMetrics(const std::map<std::string, int>& resourcesAvailable, const service::schemas::RunConfiguration* runConfiguration) const;

	void signalTasks(const std::string& signal);

	bool runResilient();
	bool run();

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection() const;

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

	std::unique_ptr<esl::com::http::server::Socket> socket;
	std::mutex socketMutex;

	mutable std::size_t nextConnectionFactory = 0;
	mutable common::plugin::ConnectionFactory* httpConnectionFactory = nullptr;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	std::size_t signalsReceived = 0;
	std::size_t signalsProcessed = 0;
	std::size_t signalsReceivedMax = 3;

	std::map<std::string, std::unique_ptr<plugin::Task>> taskByTaskId;
	std::chrono::time_point<std::chrono::steady_clock> idleTimeAt;
	std::chrono::time_point<std::chrono::steady_clock> unavailableTimeAt;
	bool availableTimeoutOccurred = false;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCEDURE_H_ */
