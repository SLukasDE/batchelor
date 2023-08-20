#ifndef BATCHELOR_WORKER_CONFIG_EVENT_H_
#define BATCHELOR_WORKER_CONFIG_EVENT_H_

#include <batchelor/common/config/Config.h>

#include <batchelor/worker/config/Setting.h>
#include <batchelor/worker/Main.h>

#include <tinyxml2/tinyxml2.h>

#include <map>
#include <string>
#include <vector>

namespace batchelor {
namespace worker {
namespace config {

class Event : public common::config::Config {
public:
	Event(Main& main, const std::string& filename, const tinyxml2::XMLElement& element);

	void setMaximumTasksRunning(std::size_t maximumTasksRunning);
	std::size_t getMaximumTasksRunning() const noexcept;

private:
	struct Setting : public config::Setting {
		Setting(Event& event, const std::string& filename, const tinyxml2::XMLElement& element);
	};
	struct Metric : public common::config::Config {
		Metric(const std::string& filename, const tinyxml2::XMLElement& element);

		std::string key;
	};

	Main& main;
	std::string id;
	std::string type;
	std::size_t maximumTasksRunning;
	std::vector<std::pair<std::string, std::string>> settings;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CONFIG_EVENT_H_ */
