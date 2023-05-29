#include <batchelor/head/Plugin.h>
#include <batchelor/head/RequestHandler.h>

#include <esl/com/http/server/RequestHandler.h>

#include <memory>

namespace batchelor {
namespace head {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"batchelor-head",
			RequestHandler::create);
}

} /* namespace head */
} /* namespace batchelor */
