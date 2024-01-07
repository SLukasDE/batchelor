#ifndef BATCHELOR_HEAD_PLUGIN_SOCKET_H_
#define BATCHELOR_HEAD_PLUGIN_SOCKET_H_

#include <esl/com/http/server/Socket.h>
#include <esl/object/Object.h>

namespace batchelor {
namespace head {
namespace plugin {

class Socket : public virtual esl::object::Object {
public:
	virtual esl::com::http::server::Socket& get() = 0;
};

} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PLUGIN_SOCKET_H_ */
