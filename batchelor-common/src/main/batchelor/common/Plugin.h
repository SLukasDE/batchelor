#ifndef BATCHELOR_COMMON_PLUGIN_H_
#define BATCHELOR_COMMON_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace batchelor {
namespace common {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_H_ */
