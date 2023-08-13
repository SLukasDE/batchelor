#ifndef BATCHELOR_HEAD_SERVICE_H_
#define BATCHELOR_HEAD_SERVICE_H_

#include <batchelor/head/Dao.h>

#include <batchelor/service/Service.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/TaskStatusHead.h>
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
	service::schemas::FetchResponse fetchTask(const service::schemas::FetchRequest& fetchRequest) override;

	std::vector<service::schemas::TaskStatusHead> getTasks(const std::string& state, const std::string& eventNotAfter, const std::string& eventNotBefore) override;

	// used by cli
	std::unique_ptr<service::schemas::TaskStatusHead> getTask(const std::string& taskId) override;
	service::schemas::RunResponse runTask(const service::schemas::RunRequest& runRequest) override;
	void sendSignal(const std::string& taskId, const std::string& signal) override;

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
