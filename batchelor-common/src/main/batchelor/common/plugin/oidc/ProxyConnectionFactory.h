#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTIONFACTORY_H_

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/client/Connection.h>

#include <memory>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class ProxyConnectionFactory : public esl::com::http::client::ConnectionFactory {
public:
	ProxyConnectionFactory(esl::com::http::client::ConnectionFactory& connectionFactory, TokenFactory& tokenFactory);

	std::unique_ptr<esl::com::http::client::Connection> createConnection() const override;

private:
	esl::com::http::client::ConnectionFactory& connectionFactory;
	TokenFactory& tokenFactory;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTIONFACTORY_H_ */
