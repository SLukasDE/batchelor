#ifndef BATCHELOR_UI_REQUESTHANDLER_H_
#define BATCHELOR_UI_REQUESTHANDLER_H_

#include <batchelor/common/plugin/ConnectionFactory.h>

#include <batchelor/ui/Procedure.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace ui {

class RequestHandler : public esl::com::http::server::RequestHandler, public esl::object::InitializeContext {
public:
	struct Settings {
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);
		Settings(const Procedure::Settings& settings);

		std::string namespaceId = "default";
		std::set<std::string> connectionFactoryIds;
	};

	RequestHandler(const Settings& settings);

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& context) override;

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection() const;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		std::vector<std::pair<std::string, std::reference_wrapper<common::plugin::ConnectionFactory>>> connectionFactories;
		//esl::com::http::client::ConnectionFactory& httpConnectionFactory;
	};

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

	mutable std::size_t nextConnectionFactory = 0;
	mutable common::plugin::ConnectionFactory* httpConnectionFactory = nullptr;
};

} /* namespace ui */
} /* namespace batchelor */

#endif /* BATCHELOR_UI_REQUESTHANDLER_H_ */
