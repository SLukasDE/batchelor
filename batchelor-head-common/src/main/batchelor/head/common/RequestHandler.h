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

#ifndef BATCHELOR_HEAD_COMMON_REQUESTHANDLER_H_
#define BATCHELOR_HEAD_COMMON_REQUESTHANDLER_H_

#include <batchelor/service/server/RequestHandler.h>

#include <batchelor/head/common/Dao.h>
#include <batchelor/head/common/Plugin.h>

#include <esl/com/http/server/RequestHandler.h>
#include <esl/database/ConnectionFactory.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace batchelor {
namespace head {
namespace common {

class RequestHandler : public service::server::RequestHandler {
public:
	RequestHandler(esl::database::ConnectionFactory& dbConnectionFactory, std::vector<std::reference_wrapper<Plugin>> plugins);
	~RequestHandler();

	esl::database::ConnectionFactory& getDbConnectionFactory() const noexcept;
	void onUpdateTask(const Dao::Task& task);

protected:
	RequestHandler();

	struct InitializedSettings {
		InitializedSettings(esl::database::ConnectionFactory& dbConnectionFactory, std::vector<std::reference_wrapper<Plugin>> plugins);

		esl::database::ConnectionFactory& dbConnectionFactory;
		std::vector<std::reference_wrapper<Plugin>> plugins;
	};

	std::unique_ptr<InitializedSettings> initializedSettings;

private:
	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	bool threadStopping = false;
	std::thread thread;

	void threadRun();
	void threadStop();
};

} /* namespace common */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_COMMON_REQUESTHANDLER_H_ */
