#ifndef BATCHELOR_HEAD_JERRYPLUGIN_PLUGIN_H_
#define BATCHELOR_HEAD_JERRYPLUGIN_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace batchelor {
namespace head {
namespace jerryplugin {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace jerryplugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_JERRYPLUGIN_PLUGIN_H_ */
