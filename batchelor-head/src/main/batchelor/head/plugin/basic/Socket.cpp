#include <batchelor/head/plugin/basic/Socket.h>

#include <esl/com/http/server/MHDSocket.h>

#include <mhd4esl/com/http/server/Socket.h>

namespace batchelor {
namespace head {
namespace plugin {
namespace basic {

std::unique_ptr<plugin::Socket> Socket::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::Socket>(new Socket(settings));
}

Socket::Socket(const std::vector<std::pair<std::string, std::string>>& settings)
: socket(new mhd4esl::com::http::server::Socket(esl::com::http::server::MHDSocket::Settings(settings)))
{ }

esl::com::http::server::Socket& Socket::get() {
	return *socket;
}

} /* namespace basic */
} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */
