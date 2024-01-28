#include <batchelor/common/plugin/basic/Socket.h>

#include <esl/com/http/server/MHDSocket.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace basic {

std::unique_ptr<plugin::Socket> Socket::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::Socket>(new Socket(settings));
}

Socket::Socket(const std::vector<std::pair<std::string, std::string>>& settings)
: socket(esl::com::http::server::MHDSocket::createNative(esl::com::http::server::MHDSocket::Settings(settings)))
{ }

esl::com::http::server::Socket& Socket::get() {
	return *socket;
}

} /* namespace basic */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
