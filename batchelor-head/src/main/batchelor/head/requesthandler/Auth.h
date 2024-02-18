#ifndef BATCHELOR_HEAD_REQUESTHANDLER_AUTH_H_
#define BATCHELOR_HEAD_REQUESTHANDLER_AUTH_H_

#include <batchelor/head/Procedure.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
namespace requesthandler {

class Auth : public esl::com::http::server::RequestHandler {
public:
	struct Settings {
		explicit Settings(const std::vector<std::pair<std::string, std::string>>& settings);
		explicit Settings(const Procedure::Settings& settings);

		std::map<std::string, Procedure::Settings::APIKeyData> apiKeys;
		std::map<std::string, Procedure::Settings::UserData> users;

		std::string realm;
	};

	Auth(const Settings& settings);

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

private:
	const Settings settings;
};

} /* namespace requesthandler */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_REQUESTHANDLER_AUTH_H_ */
