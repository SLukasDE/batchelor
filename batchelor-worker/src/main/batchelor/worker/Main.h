#ifndef BATCHELOR_WORKER_MAIN_H_
#define BATCHELOR_WORKER_MAIN_H_

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/Service.h>

#include <batchelor/worker/Options.h>
#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/system/Signal.h>
#include <esl/utility/Signal.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace batchelor {
namespace worker {

class Main {
public:
	Main(const Options& options);

	void stopRunning();
	int getReturnCode() const noexcept;

	void setMaximumTasksRunning(std::size_t maximumTasksRunning);
	std::size_t getMaximumTasksRunning() const noexcept;

	void addEventType(const std::string& id, std::unique_ptr<plugin::TaskFactory> eventType);

	void addUserDefinedMetric(const std::string& key, const std::string& value);
	std::vector<std::pair<std::string, std::string>> getCurrentMetrics() const;
	std::vector<std::pair<std::string, std::string>> getCurrentMetrics(const service::schemas::RunConfiguration& runConfiguration) const;

private:
	void signalTasks(const std::string& signal);
	void run1();
	bool run2();

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection();
	esl::com::http::client::ConnectionFactory& getHTTPConnectionFactory();

	std::size_t maximumTasksRunning = std::string::npos;
	std::vector<std::pair<std::string, std::string>> userDefinedMetrics;
	std::string url;
	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
	int rc = 0;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	std::size_t signalsReceived = 0;
	std::size_t signalsProcessed = 0;
	std::size_t signalsReceivedMax = 3;

	std::map<std::string, std::unique_ptr<plugin::Task>> taskByTaskId;
	std::map<std::string, std::unique_ptr<plugin::TaskFactory>> taskFactroyByEventType;

	std::unique_ptr<esl::system::Signal> signal;
	std::vector<esl::system::Signal::Handler> signalHandles;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_MAIN_H_ */
