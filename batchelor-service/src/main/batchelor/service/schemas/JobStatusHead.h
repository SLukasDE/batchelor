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
	std::string jobId;

	/* Id might contain following keys:
	 * - cloudId: is is 'on-premise/REWE' or 'GCP'
	 * - hostId: identifies the host the worker is running, e.g. hostname
	 * - workerId: identifies the worker, e.g. the ID of the container if multiple worker-containers are running on the same host
	 */
	// TODO: ... see fetch request
	std::vector<Setting> ids;

	RunConfiguration runConfiguration;

	/* possible values:
	 *   - waiting   // set by head   // new job has been created and is waiting to get into state running
	 *   - timeout   // set by head   // job was in state waiting but timeout occurred to change state (to running)
	 *   - running   // set by head   // job was in state waiting and has been fetched to run
	 *   - done      // set by worker // job was in state running but it has returned with a return-code
	 *   - failed    // set by worker // job was in state running but it has returned with an exception
	 * //- signaling // set by head   // job was in state running but a signal (e.g. SIGINT or SIGTERM) has been send to process
	 * //- signaled  // set by head   // job was in state signaling or waiting and job has been returned (with return-code or exception)
	 *   - zombi     // set by head   // job was in state running but worker did not send heart beat for a while
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
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_NESTED_MMEMBER(data, ids, id)
       & SERGUT_MMEMBER(data, runConfiguration)
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
