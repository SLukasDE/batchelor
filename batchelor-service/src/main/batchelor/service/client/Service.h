#ifndef BATCHELOR_SERVICE_CLIENT_SERVICE_H_
#define BATCHELOR_SERVICE_CLIENT_SERVICE_H_

#include <batchelor/service/Service.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/JobStatusHead.h>
#include <batchelor/service/schemas/JobStatusWorker.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/com/http/client/Connection.h>

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace client {

class Service : public service::Service {
public:
    Service(const esl::com::http::client::Connection& connection);

	// used by worker
	schemas::FetchResponse fetchJob(const schemas::FetchRequest& fetchRequest) override;
	std::vector<schemas::JobStatusHead> getJobs(const std::string& state) override;

	// used by cli
	std::unique_ptr<schemas::JobStatusHead> getJob(const std::string& jobId) override;
	void sendSignal(const schemas::Signal& signal) override;
	schemas::RunResponse runBatch(const schemas::RunRequest& runRequest) override;

private:
    const esl::com::http::client::Connection& connection;
};

} /* namespace client */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_CLIENT_SERVICE_H_ */
