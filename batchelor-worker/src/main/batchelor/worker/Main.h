/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#ifndef BATCHELOR_WORKER_MAIN_H_
#define BATCHELOR_WORKER_MAIN_H_

#include <batchelor/common/config/Server.h>

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/Service.h>

#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/system/Signal.h>
#include <esl/utility/Signal.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class Main final {
public:
	struct Settings {
		struct Event {
			std::string id;
			std::string type;
			std::vector<std::pair<std::string, std::string>> settings;
		};

		std::vector<std::pair<std::string, std::string>> metrics;
		std::size_t maximumTasksRunning = std::string::npos;

		std::vector<Event> events;

		// connection options
		std::vector<common::config::Server> servers;
	};

	Main(const Settings& settings);
	~Main();

	int getReturnCode() const noexcept;

	std::vector<std::pair<std::string, std::string>> getCurrentMetrics() const;
	std::vector<std::pair<std::string, std::string>> getCurrentMetrics(const service::schemas::RunConfiguration& runConfiguration) const;

private:
	void stopRunning();
	void signalTasks(const std::string& signal);

	bool runResilient();
	bool run();

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection();
	esl::com::http::client::ConnectionFactory& getHTTPConnectionFactory();

	const Settings& settings;
	std::size_t nextServer = 0;

	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
	int rc = 0;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	std::size_t signalsReceived = 0;
	std::size_t signalsProcessed = 0;
	std::size_t signalsReceivedMax = 3;

	std::map<std::string, std::unique_ptr<plugin::Task>> taskByTaskId;
	std::map<std::string, std::unique_ptr<plugin::TaskFactory>> taskFactroyByEventType;

	std::unique_ptr<esl::system::Signal> signal;
	std::vector<esl::system::Signal::Handler> signalHandles;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_MAIN_H_ */
