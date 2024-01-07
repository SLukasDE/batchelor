#ifndef BATCHELOR_HEAD_PLUGIN_BASIC_SOCKET_H_
#define BATCHELOR_HEAD_PLUGIN_BASIC_SOCKET_H_

#include <batchelor/head/plugin/Socket.h>

#include <esl/com/http/server/Socket.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
namespace plugin {
namespace basic {

class Socket : public plugin::Socket {
public:
	static std::unique_ptr<plugin::Socket> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Socket(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::com::http::server::Socket& get() override;

private:
	std::unique_ptr<esl::com::http::server::Socket> socket;
};

} /* namespace basic */
} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PLUGIN_BASIC_SOCKET_H_ */
