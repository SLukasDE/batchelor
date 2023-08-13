#include <batchelor/worker/Plugin.h>
#include <batchelor/worker/TaskFactoryExec.h>
#include <batchelor/worker/TaskFactory.h>

#include <memory>

namespace batchelor {
namespace worker {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<TaskFactory>(
			"batchelor-process-exec",
			TaskFactoryExec::create);
}

} /* namespace worker */
} /* namespace batchelor */
