#ifndef BATCHELOR_SERVICE_CLIENT_SERVICE_H_
#define BATCHELOR_SERVICE_CLIENT_SERVICE_H_

#include "batchelor/service/Service.h"
#include <batchelor/service/schemas/Procedures.h>
#include <batchelor/service/schemas/Job.h>
#include <batchelor/service/schemas/Status.h>

#include <esl/com/http/client/Connection.h>

#include <memory>

namespace batchelor {
namespace service {
namespace client {

class Service : public service::Service {
public:
    Service(const esl::com::http::client::Connection& connection);

	void setStatusCancelByWorkerId(const std::string& workerId) override;

	std::unique_ptr<schemas::Job> fetchJob(const std::string& workerId, const schemas::Procedures& ids) override;

	std::unique_ptr<schemas::Status> getStatus(const std::string& jobId) override;
	void setStatus(const std::string& jobId, const std::vector<schemas::Status>& jobs) override;

private:
    const esl::com::http::client::Connection& connection;
};

} /* namespace client */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_CLIENT_SERVICE_H_ */
