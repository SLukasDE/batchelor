#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_CONNECTIONFACTORY_H_

#include <batchelor/common/plugin/ConnectionFactory.h>
#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class ConnectionFactory : public plugin::ConnectionFactory {
public:
	static std::unique_ptr<plugin::ConnectionFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::com::http::client::ConnectionFactory& get() override;

private:
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactoryProxy;
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactoryOriginal;
	std::unique_ptr<TokenFactory> tokenFactory;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_CONNECTIONFACTORY_H_ */
