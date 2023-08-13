#ifndef BATCHELOR_WORKER_OPTIONS_H_
#define BATCHELOR_WORKER_OPTIONS_H_

#include <batchelor/common/Server.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class Options {
public:
	Options(int argc, const char* argv[]);

	void addSetting(const char* key, const char* value);
	const std::vector<std::pair<std::string, std::string>>& getSettings() const noexcept;

	void addMetrics(const char* key, const char* value);
	const std::vector<std::pair<std::string, std::string>>& getMetrics() const noexcept;

	void setCondition(const char* condition);
	const std::string& getCondition() const noexcept;

	void setMaximumTasksRunning(std::size_t maximumTasksRunning);
	std::size_t getMaximumTasksRunning() const noexcept;

	void setWait();
	bool getWait() const noexcept;

	void addConnectionFile(const char* value);
	void setUsername(const char* value);
	void setPassword(const char* value);
	void addURL(const char* value);
	const std::vector<common::Server>& getServers() const noexcept;

	static void printUsage();

private:
	std::vector<std::pair<std::string, std::string>> metrics;
	std::vector<std::pair<std::string, std::string>> settings;
	std::string condition;
	bool wait = false;
	std::size_t maximumTasksRunning = 0;

	// connection options
	std::string currentUsername;
	std::string currentPassword;
	std::vector<common::Server> servers;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_OPTIONS_H_ */
