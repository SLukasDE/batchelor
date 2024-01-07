#include <batchelor/common/plugin/oidc/ProxyConnection.h>
#include <batchelor/common/plugin/oidc/ProxyConnectionFactory.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

ProxyConnectionFactory::ProxyConnectionFactory(esl::com::http::client::ConnectionFactory& aConnectionFactory, TokenFactory& aTokenFactory)
: connectionFactory(aConnectionFactory),
  tokenFactory(aTokenFactory)
{ }

std::unique_ptr<esl::com::http::client::Connection> ProxyConnectionFactory::createConnection() const {
	return std::unique_ptr<esl::com::http::client::Connection>(new ProxyConnection(connectionFactory.createConnection(), tokenFactory));
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
