#ifndef BATCHELOR_SERVICE_SERVICE_H_
#define BATCHELOR_SERVICE_SERVICE_H_

#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/JobStatusHead.h>
#include <batchelor/service/schemas/JobStatusWorker.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/Signal.h>

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace service {

class Service {
public:
	virtual ~Service() = default;

	// used by worker
	virtual schemas::FetchResponse fetchJob(const schemas::FetchRequest& fetchRequest) = 0;

	/* possible state values:
	 *   - waiting   // set by head   // new job has been created and is waiting to get into state running
	 *   - timeout   // set by head   // job was in state waiting but timeout occurred to change state (to running)
	 *   - running   // set by head   // job was in state waiting and has been fetched to run
	 *   - done      // set by worker // job was in state running but it has returned with a return-code
	 *   - failed    // set by worker // job was in state running but it has returned with an exception
	 * //- signaling // set by head   // job was in state running but a signal (e.g. SIGINT or SIGTERM) has been send to process
	 * //- signaled  // set by head   // job was in state signaling or waiting and job has been returned (with return-code or exception)
	 *   - zombi     // set by head   // job was in state running but worker did not send heart beat for a while
	 */
	virtual std::vector<schemas::JobStatusHead> getJobs(const std::string& state) = 0;

	// used by cli
	virtual std::unique_ptr<schemas::JobStatusHead> getJob(const std::string& jobId) = 0;
	virtual void sendSignal(const schemas::Signal& signal) = 0;
	virtual schemas::RunResponse runBatch(const schemas::RunRequest& runRequest) = 0;
};

} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SERVICE_H_ */
