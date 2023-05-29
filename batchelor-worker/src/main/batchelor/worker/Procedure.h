#ifndef BATCHELOR_WORKER_PROCEDURE_H_
#define BATCHELOR_WORKER_PROCEDURE_H_

#include <batchelor/worker/Process.h>
#include <batchelor/worker/ProcessFactory.h>

#include <batchelor/service/Service.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/processing/Procedure.h>
#include <esl/utility/Signal.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
/* TODO:
 * Introduce more settings an use them:
 * - Elapsed-Seconds-To-Send-Kill-Signal
 * - Min-Interval-Seconds, Max-Interval-Seconds-Cold and Max-Interval-Seconds-Hot between Status-FetchJob-Requests are send.
 *   - Max-Interval-Seconds-Cold >= Max-Interval-Seconds-Hot
 *   - If processByJobId.empty() == false, then we are in hot-state
 *   - If processByJobId.empty() == true, then we are in cold-state
 *   - In both states dice between min-... and max-interval-seconds-... and use this time to wait.
 *   - If NO exception occurred on communication, then store current connection-factory-index as the "working index".
 *   - If exception occurred, rotate connection factory. If connection-factory-index is "working index" after rotation, then double the current used settings-value "max-intercal-seconds-..."
 *     (If the current settings-value is "max-interval-seconds-HOT" and it gets bigger then "...-COLD", then set "...-COLD" to the same value)
 */
class Procedure : public esl::processing::Procedure, public esl::object::InitializeContext {
public:
	struct Settings {
		std::set<std::string> connectionFactoryIds;
		std::map<std::string, std::string> ids;
		std::set<std::string> processFactoryIds;
		std::size_t maximumJobsRunning = 0;
	};

	struct InitializedSettings {
		InitializedSettings(const Settings& settings, esl::object::Context& context);

		std::vector<std::reference_wrapper<esl::com::http::client::ConnectionFactory>> connectionFactories;
		std::map<std::string, std::reference_wrapper<ProcessFactory>> processFactories;
	};

	Procedure(Settings settings);

	static std::unique_ptr<esl::processing::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& context) override;

	void procedureRun(esl::object::Context& context) override;

	/* this method is non-blocking. */
	void procedureCancel() override;

	void signalEvent();
	std::mutex& getMutex();

private:
	Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;
	std::size_t currentConnectionFactoryIndex = 0;

	std::mutex eventMutex;
	std::condition_variable eventCondVar;
	bool event = false;
	std::atomic<bool> eventStop { false };

	//bool unpublishedStopStatus = false;

	std::map<std::string, std::unique_ptr<Process>> processByJobId;

	/* returns:
	 * true, if successful
	 * false, if exception occurred
	 */
	bool processRequest(std::function<void(service::Service&)> processor);

	/* returns:
	 * true, if successful
	 * false, if exception occurred
	 */
	void markJobsAsZombie(service::Service& service);

	/* returns:
	 * true, if successful
	 * false, if exception occurred
	 */
	void processEvent(service::Service& service, bool stopping);

	std::size_t calculateProcessesRunning() const;
	void sendSignalToProcessesRunning(esl::utility::Signal signal) const;

	std::vector<std::pair<std::string, std::string>> createMetrics(std::size_t processesRunning);
	std::unique_ptr<esl::com::http::client::Connection> createHttpClientConnection();
	void rotateHttpClientConnection();
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCEDURE_H_ */
