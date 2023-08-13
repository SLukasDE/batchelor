#ifndef BATCHELOR_WORKER_MAIN_H_
#define BATCHELOR_WORKER_MAIN_H_

#include <batchelor/worker/Options.h>
#include <batchelor/worker/Task.h>
#include <batchelor/worker/TaskFactory.h>

#include <batchelor/service/Service.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/utility/Signal.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace batchelor {
namespace worker {

class Main {
public:
	Main(const Options& options);
	~Main();

	void stopRunning();
	int getReturnCode() const;

private:
	void signalTasks(const esl::utility::Signal& signal);
	void run1();
	bool run2();

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection();
	esl::com::http::client::ConnectionFactory& getHTTPConnectionFactory();

	const Options& options;
	std::string url;
	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
	int rc = 0;

	std::condition_variable notifyCV;
	std::mutex notifyMutex;
	bool stopThread = false;

	std::map<std::string, std::unique_ptr<Task>> taskByTaskId;
	std::map<std::string, std::unique_ptr<TaskFactory>> taskFactroyByEventType;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_MAIN_H_ */
