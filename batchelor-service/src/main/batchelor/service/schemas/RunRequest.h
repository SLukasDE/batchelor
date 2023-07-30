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
	 * - settings[2] = { 'args' ; '--propertyId=Bla --propertyFile=/wxx/secret/property.cfg' }
	 */
	std::vector<Setting> settings;

	/* First this job will go into a queue and it's state is waiting.
	 * Every time a worker is calling fetchJob, this formula will be evaluated if job is still in state waiting and if worker is offering this eventType.
	 * Available variables to get used in the formula are all variables delivered as metrics of "fetch-request" like
	 * - cpu usage              (CPU_USAGE),
	 * - memory usage           (MEM_USAGE),
	 * - number of jobs running (RUNNING_JOBS),
	 * - host name              (HOST_NAME),
	 * - cloudId,
	 * - workerId
	 * - ...
	 * as well as job specific variables provided by the head server, like
	 * - waiting time,
	 * - priority,
	 * - ...
	 * If the formula is evaluated to true, head will response to worker to run this job.
	 */
	std::string condition;
};

SERGUT_FUNCTION(RunRequest, data, ar) {
    ar & SERGUT_MMEMBER(data, eventType)
       & SERGUT_MMEMBER(data, priority)
       & SERGUT_NESTED_MMEMBER(data, settings, settings)
       & SERGUT_MMEMBER(data, condition);
}

} /* namespace schema */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_ */
