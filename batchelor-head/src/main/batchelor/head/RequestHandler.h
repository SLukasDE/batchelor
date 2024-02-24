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

#ifndef BATCHELOR_HEAD_REQUESTHANDLER_H_
#define BATCHELOR_HEAD_REQUESTHANDLER_H_

#include <batchelor/head/Dao.h>
#include <batchelor/head/Engine.h>
#include <batchelor/head/plugin/Observer.h>
#include <batchelor/head/Procedure.h>

#include <batchelor/service/server/RequestHandler.h>

#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/database/Connection.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>


namespace batchelor {
namespace head {

class RequestHandler : public Engine, public service::server::RequestHandler, public esl::object::InitializeContext {
public:
	struct Settings {
		explicit Settings(const std::vector<std::pair<std::string, std::string>>& settings);
		explicit Settings(const Procedure::Settings& settings);

		// after how many seconds do we handle a task or worker as zombie?
		std::chrono::milliseconds timeoutZombie{0};

		// after how many seconds can we delete old stuff?
		std::chrono::milliseconds timeoutCleanup{0};

		std::string dbConnectionFactoryId;
		std::set<std::string> pluginIds;
	};

	RequestHandler(const Settings& settings);
	~RequestHandler();

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& context) override;

	esl::database::ConnectionFactory& getDbConnectionFactory() const noexcept override;
	void onUpdateTask(const Dao::Task& task) override;

private:
	struct InitializedSettings {
	private:
		std::unique_ptr<esl::database::ConnectionFactory> dbConnectionFactoryPtr;

	public:
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		esl::database::ConnectionFactory& dbConnectionFactory;
		std::vector<std::reference_wrapper<plugin::Observer>> plugins;
	};

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

	std::condition_variable notifyCV;
	mutable std::mutex notifyMutex;
	bool threadStopping = false;
	std::thread thread;

	void threadRun();
	void threadStop();
	void cleanup();
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_REQUESTHANDLER_H_ */
