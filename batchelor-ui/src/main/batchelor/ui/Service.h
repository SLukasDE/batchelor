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

#ifndef BATCHELOR_UI_SERVICE_H_
#define BATCHELOR_UI_SERVICE_H_

#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/TaskStatusHead.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

#include <batchelor/service/Service.h>

#include <batchelor/ui/RequestHandler.h>

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace ui {

class Service : public service::Service {
public:
    Service(const RequestHandler& requestHandler);

	void alive() override;

	// used by worker
	service::schemas::FetchResponse fetchTask(const std::string& namespaceId, const service::schemas::FetchRequest& fetchRequest) override;

	// used by controller-cli
	std::vector<service::schemas::TaskStatusHead> getTasks(const std::string& namespaceId, const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) override;

	// used by controller-cli
	std::unique_ptr<service::schemas::TaskStatusHead> getTask(const std::string& namespaceId, const std::string& taskId) override;

	// used by controller-cli
	service::schemas::RunResponse runTask(const std::string& namespaceId, const service::schemas::RunRequest& runRequest) override;

	// used by controller-cli
	void sendSignal(const std::string& namespaceId, const std::string& taskId, const std::string& signal) override;

	// used by controller-cli
	std::vector<std::string> getEventTypes(const std::string& namespaceId) override;

private:
	const RequestHandler& requestHandler;
};

} /* namespace ui */
} /* namespace batchelor */

#endif /* BATCHELOR_UI_SERVICE_H_ */
