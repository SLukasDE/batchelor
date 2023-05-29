#ifndef BATCHELOR_WORKER_PLUGIN_H_
#define BATCHELOR_WORKER_PLUGIN_H_

#include <esl/plugin/Registry.h>

namespace batchelor {
namespace worker {

class Plugin final {
public:
	Plugin() = delete;
	static void install(esl::plugin::Registry& registry, const char* data);
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_H_ */
