#ifndef BATCHELOR_HEAD_SERVICE_H_
#define BATCHELOR_HEAD_SERVICE_H_

#include <batchelor/head/Dao.h>

#include <batchelor/service/Service.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/JobStatusHead.h>
#include <batchelor/service/schemas/JobStatusWorker.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/database/Connection.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/object/Context.h>

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace head {

class Service : public service::Service {
public:
	Service(const esl::object::Context& context, esl::database::ConnectionFactory& dbConnectionFactory);

	// used by worker
	service::schemas::FetchResponse fetchJob(const service::schemas::FetchRequest& fetchRequest) override;

	std::vector<service::schemas::JobStatusHead> getJobs(const std::string& state) override;

	// used by cli
	std::unique_ptr<service::schemas::JobStatusHead> getJob(const std::string& jobId) override;
	void sendSignal(const service::schemas::Signal& signal) override;
	service::schemas::RunResponse runBatch(const service::schemas::RunRequest& runRequest) override;

private:
	esl::database::Connection& getDBConnection() const;
	Dao& getDao() const;

	const esl::object::Context& context;
	esl::database::ConnectionFactory& dbConnectionFactory;
	mutable std::unique_ptr<esl::database::Connection> dbConnection;
	mutable std::unique_ptr<Dao> dao;
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_SERVICE_H_ */
