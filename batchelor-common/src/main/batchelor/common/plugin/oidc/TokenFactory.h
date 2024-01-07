#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORY_H_

#include <esl/com/http/client/Response.h>

#include <chrono>
#include <string>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class TokenFactory {
public:
	TokenFactory() = default;
	virtual ~TokenFactory() = default;

	virtual std::string getToken() = 0;

protected:
	void processResponse(const esl::com::http::client::Response& response, const std::string& payload);

	std::string tokenType;

	std::string accessToken;
	std::chrono::time_point<std::chrono::steady_clock> accessTokenExpiresAt;

	std::string refreshToken;
	std::chrono::time_point<std::chrono::steady_clock> refreshTokenExpiresAt;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORY_H_ */
