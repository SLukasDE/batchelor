#include <batchelor/common/Plugin.h>
#include <batchelor/common/plugin/basic/ConnectionFactory.h>
#include <batchelor/common/plugin/ConnectionFactory.h>
#include <batchelor/common/plugin/oidc/ConnectionFactory.h>
#include <batchelor/common/plugin/wrapper/ConnectionFactory.h>

#include <esl/object/Object.h>

namespace batchelor {
namespace common {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin("basic", plugin::basic::ConnectionFactory::create);
	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::basic::ConnectionFactory::create>("batchelor-connection-basic");

	registry.addPlugin("oidc", plugin::oidc::ConnectionFactory::create);
	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::oidc::ConnectionFactory::create>("batchelor-connection-oidc");

	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::wrapper::ConnectionFactory::create>("batchelor-connection-wrapper");
}

} /* namespace common */
} /* namespace batchelor */
