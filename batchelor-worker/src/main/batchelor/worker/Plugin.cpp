#include <batchelor/worker/Plugin.h>
#include <batchelor/worker/plugin/exec/TaskFactory.h>
#include <batchelor/worker/plugin/kubectl/TaskFactory.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <memory>

namespace batchelor {
namespace worker {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<plugin::TaskFactory>(
			"exec",
			plugin::exec::TaskFactory::create);

	registry.addPlugin<plugin::TaskFactory>(
			"kubectl",
			plugin::kubectl::TaskFactory::create);
}

} /* namespace worker */
} /* namespace batchelor */
