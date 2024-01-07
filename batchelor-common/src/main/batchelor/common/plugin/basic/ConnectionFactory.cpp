#include <batchelor/common/plugin/basic/ConnectionFactory.h>

#include <curl4esl/com/http/client/ConnectionFactory.h>

#include <esl/com/http/client/CURLConnectionFactory.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace basic {

std::unique_ptr<plugin::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::ConnectionFactory>(new ConnectionFactory(settings));
}

ConnectionFactory::ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings)
: connectionFactory(new curl4esl::com::http::client::ConnectionFactory(esl::com::http::client::CURLConnectionFactory::Settings(settings)))
{ }

esl::com::http::client::ConnectionFactory& ConnectionFactory::get() {
	return *connectionFactory;
}

} /* namespace basic */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
