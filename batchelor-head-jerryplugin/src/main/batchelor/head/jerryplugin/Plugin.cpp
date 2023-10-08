#include <batchelor/head/jerryplugin/Plugin.h>

#include <batchelor/head/jerryplugin/RequestHandler.h>

#include <esl/com/http/server/RequestHandler.h>

#include <memory>

namespace batchelor {
namespace head {
namespace jerryplugin {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"batchelor-head",
			RequestHandler::create);
}

} /* namespace jerryplugin */
} /* namespace head */
} /* namespace batchelor */
