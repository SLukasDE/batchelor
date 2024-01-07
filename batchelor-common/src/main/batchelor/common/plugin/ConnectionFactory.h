#ifndef BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/object/Object.h>

namespace batchelor {
namespace common {
namespace plugin {

class ConnectionFactory : public esl::object::Object {
public:
	virtual esl::com::http::client::ConnectionFactory& get() = 0;
};

} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_CONNECTIONFACTORY_H_ */
