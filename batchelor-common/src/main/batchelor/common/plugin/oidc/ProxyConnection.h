#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/Output.h>

#include <functional>
#include <memory>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class ProxyConnection : public esl::com::http::client::Connection {
public:
	ProxyConnection(std::unique_ptr<esl::com::http::client::Connection> connection, TokenFactory& tokenFactory);

	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const override;
	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, esl::io::Input input) const override;

private:
	std::unique_ptr<esl::com::http::client::Connection> connection;
	TokenFactory& tokenFactory;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_ */
