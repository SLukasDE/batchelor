#ifndef BATCHELOR_CONTROL_MAIN_H_
#define BATCHELOR_CONTROL_MAIN_H_

#include <batchelor/control/Options.h>

#include <batchelor/service/schemas/TaskStatusHead.h>
#include <batchelor/service/Service.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/system/Signal.h>
#
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace batchelor {
namespace control {

class Main {
public:
	Main(const Options& options);

	void stopRunning();

	void sendEvent();
	void waitTask(const std::string& taskId);
	void signalTask(const std::string& taskId, const std::string& signal);
	void showTask();
	void showTasks();

	int getReturnCode() const;

private:
	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection() const;
	esl::com::http::client::ConnectionFactory& getHTTPConnectionFactory() const;
	void showTask(const service::schemas::TaskStatusHead& taskStatus) const noexcept;

	const Options& options;
	std::string url;
	mutable std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
	int rc = 0;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	int signalsReceived = 0;
	int signalsProcessed = 0;
	std::unique_ptr<esl::system::Signal> signal;
	std::vector<esl::system::Signal::Handler> signalHandles;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_MAIN_H_ */
