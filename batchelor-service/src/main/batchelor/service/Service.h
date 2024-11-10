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

#ifndef BATCHELOR_SERVICE_SERVICE_H_
#define BATCHELOR_SERVICE_SERVICE_H_

#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/TaskStatusHead.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace service {

class Service {
public:
	virtual ~Service() = default;

	virtual void alive() = 0;

	/* This is a complex call used by workers.
	 * Workers are sending by FetchRequest
	 * - the state of their current running tasks.
	 * - available tasks to execute
	 * - ...
	 * Workers will receive by FetchResponse
	 * - what they have to execute
	 * - what signal has to send to a specific task
	 * - ...
	 */
	virtual schemas::FetchResponse fetchTask(const std::string& namespaceId, const schemas::FetchRequest& fetchRequest) = 0;

	/* possible state values:
	 * - waiting   // set by head   // new task has been created and is waiting to get into state running
	 * - timeout   // set by head   // task was in state waiting but timeout occurred to change state (to running)
	 * - running   // set by head   // task was in state waiting and has been fetched to run
	 * - done      // set by worker // task was in state running but it has returned with a return-code
	 * - failed    // set by worker // task was in state running but it has returned with an exception
	 * - zombi     // set by head   // task was in state running but worker did not send heart beat for a while
	 */
	/* This call is used by a controller-cli or a web frontend to get a list of all tasks
	 * - that are waiting,
	 * - could not switch to running because of a timeout,
	 * - that are running, done or failed
	 * - died as zombie because worker died.
	 * Each entry contains the task id, arguments/settings, state, return-code or error-message, start time, stop time, ...
	 */
	virtual std::vector<schemas::TaskStatusHead> getTasks(const std::string& namespaceId, const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) = 0;

	/* This call is used by a controller-cli or a web frontend to get data of a specific task.
	 * It is almost the same service as above, but now for a specific task id.
	 */
	virtual std::unique_ptr<schemas::TaskStatusHead> getTask(const std::string& namespaceId, const std::string& taskId) = 0;

	/* This call is used by a controller-cli or a web frontend */
	virtual schemas::RunResponse runTask(const std::string& namespaceId, const schemas::RunRequest& runRequest) = 0;

	/* This call is used by a controller-cli or a web frontend to send a specific signal to a task or to cancel the task */
	virtual void sendSignal(const std::string& namespaceId, const std::string& taskId, const std::string& signal) = 0;

	/* This call is used by a controller-cli or a web frontend to get a list of available event types */
	virtual std::vector<std::string> getEventTypes(const std::string& namespaceId) = 0;
};

} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SERVICE_H_ */
