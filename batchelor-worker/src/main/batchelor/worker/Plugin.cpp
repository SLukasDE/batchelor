#include <batchelor/worker/Plugin.h>
#include <batchelor/worker/Procedure.h>
#include <batchelor/worker/ProcessExecFactory.h>
#include <batchelor/worker/ProcessFactory.h>

#include <esl/object/Object.h>
#include <esl/processing/Procedure.h>

#include <memory>

namespace batchelor {
namespace worker {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<esl::processing::Procedure>(
			"batchelor-worker",
			Procedure::create);

	registry.addPlugin<esl::object::Object>(
			"batchelor-process-exec",
			ProcessExecFactory::createObject);

	registry.addPlugin<ProcessFactory>(
			"batchelor-process-exec",
			ProcessExecFactory::create);
}

} /* namespace worker */
} /* namespace batchelor */
