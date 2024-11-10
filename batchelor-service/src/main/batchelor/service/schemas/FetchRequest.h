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

#ifndef BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_
#define BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_

#include <batchelor/service/schemas/EventTypeAvailable.h>
#include <batchelor/service/schemas/TaskStatusWorker.h>
#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct FetchRequest {
	std::string workerId;

	/* List of available event types and a flag, if event type is possible to use in FetchResposne.
	 *
	 * A request is still valid with multiple entries with same value, even if it makes no sense. But the receiver should be able to handle this.
	 * Because of this the list is a vector and not a set.
	 */
	std::vector<EventTypeAvailable> eventTypes;

	/* Metrics contains all metric variables and their values. These are built-in variables, like
	 * - cpu usage               (CPU_USAGE),
	 * - memory usage            (MEM_USAGE),
	 * - number of running tasks (TASKS_RUNNING),
	 * - host name               (HOST_NAME)
	 * But there are also user defined variables, like
	 * - cloudId (identifies on which cloud the worker is running. Values could be like 'OnPrem' or 'GCP')
	 * - workerId (value identifies the worker, e.g. the ID of the container if multiple worker-containers are running on the same host)
	 */
	// TODO: Should it be an map instead of vector? -> Depends... There should be no request with multiple keys and we must handle it by sending an error response.
	// What is the behavior of the deserializer if it receives multiple key entries for a map? If it throws an exception then we can catch it an send an error response.
	std::vector<Setting> metrics;

	std::vector<TaskStatusWorker> tasks;
};

SERGUT_FUNCTION(FetchRequest, data, ar) {
    ar & SERGUT_MMEMBER(data, workerId)
       & SERGUT_NESTED_MMEMBER(data, eventTypes, eventTypes)
       & SERGUT_NESTED_MMEMBER(data, metrics, metric)
       & SERGUT_NESTED_MMEMBER(data, tasks, tasks);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_ */
