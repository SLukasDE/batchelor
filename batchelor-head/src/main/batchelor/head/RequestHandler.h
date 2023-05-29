#ifndef BATCHELOR_HEAD_REQUESTHANDLER_H_
#define BATCHELOR_HEAD_REQUESTHANDLER_H_

#include <batchelor/service/server/RequestHandler.h>

#include <esl/com/http/server/RequestHandler.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {

class RequestHandler : public service::server::RequestHandler, public esl::object::InitializeContext {
public:
	struct Settings {
		std::string dbConnectionFactoryId;
	};

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	RequestHandler(Settings settings);
	RequestHandler(esl::database::ConnectionFactory& dbConnectionFactory);

	void initializeContext(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);
		InitializedSettings(esl::database::ConnectionFactory& dbConnectionFactory);

		esl::database::ConnectionFactory& dbConnectionFactory;
	};

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_REQUESTHANDLER_H_ */
