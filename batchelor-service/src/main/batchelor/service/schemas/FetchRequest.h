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
	 */
	// TODO: Should it be a set instead of vector? -> Hmmm...no? Request is still valid with multiple entries.
	std::vector<std::string> batchIds;

	/* Id might contain following keys:
	 * - cloudId: is is 'on-premise/REWE' or 'GCP'
	 * - hostId: identifies the host that the worker is running on, e.g. hostname
	 * - workerId: identifies the worker, e.g. the ID of the container if multiple worker-containers are running on the same host
	 */
	// TODO: Should it be an map instead of vector? -> Depends... There should be no request with multiple keys and we must handle it by sending an error response.
	// What is the behavior of the deserializer if it receives multiple key entries for a map? If it throws an exception then we can catch it an send an error resposne.
	std::vector<Setting> ids;

	std::vector<Setting> metrics;

	std::vector<JobStatusWorker> jobs;
};

SERGUT_FUNCTION(FetchRequest, data, ar) {
    ar & SERGUT_NESTED_MMEMBER(data, ids, id)
       & SERGUT_NESTED_MMEMBER(data, metrics, metric)
       & SERGUT_NESTED_MMEMBER(data, batchIds, batchId)
       & SERGUT_NESTED_MMEMBER(data, jobs, status);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_FETCHREQUEST_H_ */
