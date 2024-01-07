#include <batchelor/common/plugin/oidc/ProxyConnection.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

ProxyConnection::ProxyConnection(std::unique_ptr<esl::com::http::client::Connection> aConnection, TokenFactory& aTokenFactory)
: connection(std::move(aConnection)),
  tokenFactory(aTokenFactory)
{
	if(!connection) {
		throw std::runtime_error("HTTP connection is empty");
	}
}

esl::com::http::client::Response ProxyConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const {
	esl::com::http::client::Request request(aRequest);
	request.addHeader("Authorization", "Bearer " + tokenFactory.getToken());

	return connection->send(request, std::move(output), createInput);
}

esl::com::http::client::Response ProxyConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, esl::io::Input input) const {
	esl::com::http::client::Request request(aRequest);
	request.addHeader("Authorization", "Bearer " + tokenFactory.getToken());

	return connection->send(request, std::move(output), std::move(input));
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
