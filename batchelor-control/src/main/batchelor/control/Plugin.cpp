#include <batchelor/common/Plugin.h>

//#include <batchelor/control/Main.h>
#include <batchelor/control/Plugin.h>

#include <memory>

namespace batchelor {
namespace control {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	common::Plugin::install(esl::plugin::Registry::get(), nullptr);

	//registry.addPlugin("batchelor-worker", Main::create);
}

} /* namespace control */
} /* namespace batchelor */
