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

#ifndef BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSWORKER_H_
#define BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSWORKER_H_

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct TaskStatusWorker {
	std::string taskId;

	/* possible values:
	 * - running   // set by head   // task was in state waiting and head returned this task on fetch request
	 * - done      // set by worker // task was in state running but it has returned with a return-code
	 * - failed    // set by worker // task was in state running but it has returned with an exception
	 */
	std::string state;

	int returnCode;
	std::string message; // e.g. exception message
};

SERGUT_FUNCTION(TaskStatusWorker, data, ar) {
    ar & SERGUT_MMEMBER(data, taskId)
       & SERGUT_MMEMBER(data, state)
       & SERGUT_MMEMBER(data, returnCode)
       & SERGUT_MMEMBER(data, message);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSWORKER_H_ */
