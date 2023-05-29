#ifndef BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSWORKER_H_
#define BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSWORKER_H_

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct JobStatusWorker {
	std::string jobId;

	/* possible values:
	 * - running   // set by head   // job was in state waiting and head returned this job on fetch request
	 * - done      // set by worker // job was in state running but it has returned with a return-code
	 * - failed    // set by worker // job was in state running but it has returned with an exception
	 */
	std::string state;

	int returnCode;
	std::string message; // e.g. exception message
};

SERGUT_FUNCTION(JobStatusWorker, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, state)
       & SERGUT_MMEMBER(data, returnCode)
       & SERGUT_MMEMBER(data, message);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_JOBSTATUSWORKER_H_ */
