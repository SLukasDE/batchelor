#ifndef BATCHELOR_HEAD_PLUGIN_H_
#define BATCHELOR_HEAD_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace batchelor {
namespace head {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_PLUGIN_H_ */
