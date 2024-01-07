#ifndef BATCHELOR_CONTROL_PLUGIN_H_
#define BATCHELOR_CONTROL_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace batchelor {
namespace control {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_PLUGIN_H_ */
