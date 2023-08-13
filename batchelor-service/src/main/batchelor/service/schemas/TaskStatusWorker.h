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
