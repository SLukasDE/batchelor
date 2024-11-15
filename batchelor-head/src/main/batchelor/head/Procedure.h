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

#ifndef BATCHELOR_HEAD_PROCEDURE_H_
#define BATCHELOR_HEAD_PROCEDURE_H_

#include <batchelor/common/auth/UserData.h>
#include <batchelor/common/Procedure.h>
#include <batchelor/common/plugin/Socket.h>

#include <batchelor/head/plugin/Observer.h>

#include <esl/com/http/server/RequestHandler.h>
#include <esl/object/Context.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace batchelor {
namespace head {

class Procedure : public common::Procedure {
public:
	struct Settings {
		Settings() = default;

		std::map<std::string, common::auth::UserData> users;

		std::map<std::string, std::string> userByPlainApiKey;
		std::map<std::string, std::string> plainBasicAuthByUser;

		// after how many seconds do we handle a task or worker as zombie?
		std::chrono::seconds timeoutZombie = std::chrono::minutes(5);

		// after how many seconds can we delete old stuff?
		std::chrono::seconds timeoutCleanup = std::chrono::hours(1);

		std::set<std::string> observerIds;
		std::set<std::string> socketIds;
		std::string databaseId = "batchelor-db";
	};

	Procedure(const Settings& settings);
	~Procedure();

	void procedureCancel() override;
	void initializeContext(esl::object::Context& context) override;

protected:
	void internalProcedureRun(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		std::map<std::string, std::reference_wrapper<plugin::Observer>> observers;
		std::map<std::string, std::reference_wrapper<common::plugin::Socket>> sockets;
		std::unique_ptr<esl::com::http::server::RequestHandler> requestHandler;
	};

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;
	std::mutex mutex;
	std::atomic<unsigned int> listeners{0};
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PROCEDURE_H_ */
