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

#ifndef BATCHELOR_HEAD_SERVICE_H_
#define BATCHELOR_HEAD_SERVICE_H_

#include <batchelor/head/Dao.h>
#include <batchelor/head/Engine.h>
#include <batchelor/head/Procedure.h>

#include <batchelor/service/Service.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/TaskStatusHead.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/database/Connection.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/object/Context.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace batchelor {
namespace head {

class Service : public service::Service {
public:
	Service(const esl::object::Context& context, Engine& engine, std::mutex& mutex);

	void alive() override;

	// used by worker
	service::schemas::FetchResponse fetchTask(const std::string& namespaceId, const service::schemas::FetchRequest& fetchRequest) override;

	std::vector<service::schemas::TaskStatusHead> getTasks(const std::string& namespaceId, const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) override;

	// used by cli
	std::unique_ptr<service::schemas::TaskStatusHead> getTask(const std::string& namespaceId, const std::string& taskId) override;
	service::schemas::RunResponse runTask(const std::string& namespaceId, const service::schemas::RunRequest& runRequest) override;
	void sendSignal(const std::string& namespaceId, const std::string& taskId, const std::string& signal) override;
	std::vector<std::string> getEventTypes(const std::string& namespaceId) override;

private:
	esl::database::Connection& getDBConnection() const;
	Dao& getDao() const;
//	std::set<Procedure::Settings::Role> getRoles(const std::string& namespaceId);

	const esl::object::Context& context;
	Engine& engine;
	std::lock_guard<std::mutex> lockMutex;
	mutable std::unique_ptr<esl::database::Connection> dbConnection;
	mutable std::unique_ptr<Dao> dao;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_SERVICE_H_ */
