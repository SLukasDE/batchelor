#ifndef BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_
#define BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_

#include <batchelor/service/schemas/JobStatusWorker.h>
#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct FetchRequest {
	/* List of procedure-id's possible to use in FetchResposne.
	 * If worker is busy but just wants to send an heart beat with status information, this list is empty.
	 *
	 * A request is still valid with multiple entries with same value, even if it makes no sense. But the receiver should be able to handle this.
	 * Because of this the list is a vector and not a set.
	 */
	std::vector<std::string> eventTypes;

	/* Metrics contains all metric variables and their values. These are built-in variables, like
	 * - cpu usage              (CPU_USAGE),
	 * - memory usage           (MEM_USAGE),
	 * - number of jobs running (RUNNING_JOBS),
	 * - host name              (HOST_NAME)
	 * But there are also user defined variables, like
	 * - cloudId (identifies on which cloud the worker is running. Values could be like 'REWE-OnPrem' or 'REWE-GCP')
	 * - workerId (value identifies the worker, e.g. the ID of the container if multiple worker-containers are running on the same host)
	 */
	// TODO: Should it be an map instead of vector? -> Depends... There should be no request with multiple keys and we must handle it by sending an error response.
	// What is the behavior of the deserializer if it receives multiple key entries for a map? If it throws an exception then we can catch it an send an error response.
	std::vector<Setting> metrics;

	std::vector<JobStatusWorker> jobs;
};

SERGUT_FUNCTION(FetchRequest, data, ar) {
    ar & SERGUT_NESTED_MMEMBER(data, eventTypes, eventTypes)
       & SERGUT_NESTED_MMEMBER(data, metrics, metric)
       & SERGUT_NESTED_MMEMBER(data, jobs, jobs);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_ */
