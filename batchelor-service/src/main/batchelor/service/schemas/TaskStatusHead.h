#ifndef BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSHEAD_H_
#define BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSHEAD_H_

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct TaskStatusHead {
	RunConfiguration runConfiguration;

	/* Metrics contains all metric variables and their values as used for the condition at the time the task has been assigned to a worker and state changed to running.
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
	 */
	std::vector<Setting> metrics;

	/* possible values:
	 * - waiting   // set by head   // new task has been created and is waiting to get into state running
	 * - timeout   // set by head   // task was in state waiting but timeout occurred to change state (to running)
	 * - running   // set by head   // task was in state waiting and has been fetched to run
	 * - done      // set by worker // task was in state running but it has returned with a return-code
	 * - failed    // set by worker // task was in state running but it has returned with an exception
	 * - zombi     // set by head   // task was in state running but worker did not send heart beat for a while
	 */
	std::string state;

	int returnCode;
	std::string message; // e.g. exception message

	std::string tsCreated;
	std::string tsRunning;
	std::string tsFinished;
	std::string tsLastHeartBeat;
};

SERGUT_FUNCTION(TaskStatusHead, data, ar) {
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

#endif /* BATCHELOR_SERVICE_SCHEMAS_TASKSTATUSHEAD_H_ */
