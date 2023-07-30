#ifndef BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSHEAD_H_
#define BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSHEAD_H_

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct JobStatusHead {
	RunConfiguration runConfiguration;

	/* Metrics contains all metric variables and their values as used for the condition at the time the job has been assigned to a worker and state changed to running.
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
	 */
	std::vector<Setting> metrics;

	/* possible values:
	 * - waiting   // set by head   // new job has been created and is waiting to get into state running
	 * - timeout   // set by head   // job was in state waiting but timeout occurred to change state (to running)
	 * - running   // set by head   // job was in state waiting and has been fetched to run
	 * - done      // set by worker // job was in state running but it has returned with a return-code
	 * - failed    // set by worker // job was in state running but it has returned with an exception
	 * - zombi     // set by head   // job was in state running but worker did not send heart beat for a while
	 */
	std::string state;

	int returnCode;
	std::string message; // e.g. exception message

	std::string tsCreated;
	std::string tsRunning;
	std::string tsFinished;
	std::string tsLastHeartBeat;
};

SERGUT_FUNCTION(JobStatusHead, data, ar) {
    ar & SERGUT_MMEMBER(data, runConfiguration)
       & SERGUT_NESTED_MMEMBER(data, metrics, metric)
       & SERGUT_MMEMBER(data, state)
       & SERGUT_MMEMBER(data, returnCode)
       & SERGUT_MMEMBER(data, message)
       & SERGUT_MMEMBER(data, tsCreated)
       & SERGUT_MMEMBER(data, tsRunning)
       & SERGUT_MMEMBER(data, tsFinished)
       & SERGUT_MMEMBER(data, tsLastHeartBeat);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSHEAD_H_ */
