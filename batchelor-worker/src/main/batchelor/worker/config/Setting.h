#ifndef BATCHELOR_WORKER_CONFIG_SETTING_H_
#define BATCHELOR_WORKER_CONFIG_SETTING_H_

#include <batchelor/common/config/Config.h>

#include <batchelor/worker/Main.h>

#include <tinyxml2/tinyxml2.h>

namespace batchelor {
namespace worker {
namespace config {

struct Setting : public common::config::Config {
	Setting(const std::string& filename, const tinyxml2::XMLElement& element, bool allowLanguage = true);

	std::string key;
	std::string value;
	std::string language;
};

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CONFIG_SETTING_H_ */
