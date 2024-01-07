#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>

#include <memory>
#include <string>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class TokenFactoryClientCredentials : public TokenFactory {
public:
	TokenFactoryClientCredentials(std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory, const std::string& clientId, const std::string& clientSecret);

	std::string getToken() override;

private:
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory;
	const std::string clientId;
	const std::string clientSecret;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_ */
