#ifndef BATCHELOR_WORKER_CONFIG_CONFIG_H_
#define BATCHELOR_WORKER_CONFIG_CONFIG_H_

#include <batchelor/common/config/Config.h>

#include <batchelor/worker/config/Setting.h>
#include <batchelor/worker/Main.h>

#include <tinyxml2/tinyxml2.h>

#include <string>

namespace batchelor {
namespace worker {
namespace config {

class Config : public common::config::Config {
public:
	Config(Main& main, const std::string& filename);

	Main& getMain() const noexcept;

private:
	struct Setting : public config::Setting {
		Setting(Main& main, const std::string& filename, const tinyxml2::XMLElement& element);
	};
	struct UserDefinedMetrics : public config::Setting {
		UserDefinedMetrics(Main& main, const std::string& filename, const tinyxml2::XMLElement& element);
	};

	Main& main;
	std::size_t maxTasksRunning = 0;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CONFIG_CONFIG_H_ */
