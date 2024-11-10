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

#ifndef BATCHELOR_CONTROL_PROCEDURE_H_
#define BATCHELOR_CONTROL_PROCEDURE_H_

#include <batchelor/common/Procedure.h>
#include <batchelor/common/plugin/ConnectionFactory.h>

#include <batchelor/control/Command.h>

#include <batchelor/service/schemas/TaskStatusHead.h>
#include <batchelor/service/Service.h>

#include <esl/com/http/client/Connection.h>
#include <esl/object/Context.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace control {

class Procedure : public common::Procedure {
public:
	struct Connection {
		std::string plugin;
		std::vector<std::pair<std::string, std::string>> settings;
	};

	struct Settings {
		std::string namespaceId;
		std::unique_ptr<Command> command;
		std::string eventType;
		int priority = -1;
		std::vector<std::pair<std::string, std::string>> metrics;
		std::vector<std::pair<std::string, std::string>> settings;
		std::string condition;
		bool wait = false;
		int waitCancel = -2;
		std::string taskId;
		std::string signal;
		std::string state;
		std::string eventNotAfter;
		std::string eventNotBefore;
		std::set<std::string> connectionFactoryIds;
	};

	Procedure(const Settings& settings);

	void procedureCancel() override;
	void initializeContext(esl::object::Context& context) override;

	int getReturnCode() const noexcept;

protected:
	void internalProcedureRun(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		std::vector<std::pair<std::string, std::reference_wrapper<common::plugin::ConnectionFactory>>> connectionFactories;
	};

	void sendEvent();
	void waitTask(const std::string& taskId);
	void signalTask(const std::string& taskId, const std::string& signal);
	void showTask();
	void showTasks();
	void showEventTypes();

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection() const;
	void showTask(const service::schemas::TaskStatusHead& taskStatus) const noexcept;

	const Settings& settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

	mutable std::size_t nextConnectionFactory = 0;
	mutable common::plugin::ConnectionFactory* httpConnectionFactory = nullptr;
	int rc = 0;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	int signalsReceived = 0;
	int signalsProcessed = 0;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_PROCEDURE_H_ */
