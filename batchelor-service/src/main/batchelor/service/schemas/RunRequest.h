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

#ifndef BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_
#define BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_

#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct RunRequest {
	std::string eventType;
	unsigned int priority;
	/* contains event type specific settings, like
	 * - arguments,
	 * - environment variables,
	 * - working dir, ...
	 * e.g.:
	 * - settings[0] = { 'env' ; 'DISPLAY=0' }
	 * - settings[1] = { 'env' ; 'TMP_DIR=/tmp' }
	 * - settings[2] = { 'args' ; '--propertyId=Bla --propertyFile=/etc/secret/test.pwd' }
	 */
	std::vector<Setting> settings;

	std::vector<Setting> metrics;

	/* First this task will go into a queue and it's state is waiting.
	 * Every time a worker is calling fetchTask, this formula will be evaluated if task is still in state waiting and if worker is offering this eventType.
	 * Available variables to get used in the formula are all variables delivered as metrics of "fetch-request" like
	 * - cpu usage               (CPU_USAGE),
	 * - memory usage            (MEM_USAGE),
	 * - number of running tasks (TASKS_RUNNING),
	 * - host name               (HOST_NAME),
	 * - cloudId,
	 * - workerId
	 * - ...
	 * as well as task specific variables provided by the head server, like
	 * - waiting time,
	 * - priority,
	 * - ...
	 * If the formula is evaluated to true, head will response to worker to run this task.
	 */
	std::string condition;
};

SERGUT_FUNCTION(RunRequest, data, ar) {
    ar & SERGUT_MMEMBER(data, eventType)
       & SERGUT_MMEMBER(data, priority)
       & SERGUT_NESTED_MMEMBER(data, settings, settings)
       & SERGUT_NESTED_MMEMBER(data, metrics, metrics)
       & SERGUT_MMEMBER(data, condition);
}

} /* namespace schema */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_ */
