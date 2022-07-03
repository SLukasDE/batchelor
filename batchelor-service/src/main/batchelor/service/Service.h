#ifndef BATCHELOR_SERVICE_SERVICE_H_
#define BATCHELOR_SERVICE_SERVICE_H_

#include <batchelor/service/schemas/Procedures.h>
#include <batchelor/service/schemas/Job.h>
#include <batchelor/service/schemas/Status.h>

#include <memory>

namespace batchelor {
namespace service {

class Service {
public:
	virtual ~Service() = default;

	virtual void setStatusCancelByWorkerId(const std::string& workerId) = 0;

	virtual std::unique_ptr<schemas::Job> fetchJob(const std::string& workerId, const schemas::Procedures& ids) = 0;

	virtual std::unique_ptr<schemas::Status> getStatus(const std::string& jobId) = 0;
	virtual void setStatus(const std::string& jobId, const std::vector<schemas::Status>& jobs) = 0;
};

} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SERVICE_H_ */
