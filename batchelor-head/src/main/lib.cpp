#include <batchelor/head/Plugin.h>

#include <esl/plugin/Registry.h>

extern "C" void esl__plugin__library__install(esl::plugin::Registry* registry, const char* data) {
	if(registry != nullptr) {
		batchelor::head::Plugin::install(*registry, data);
	}
}
