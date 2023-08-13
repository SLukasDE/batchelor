#ifndef BATCHELOR_CONTROL_MAIN_H_
#define BATCHELOR_CONTROL_MAIN_H_

#include <batchelor/control/Options.h>

#include <batchelor/service/Service.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>

#include <memory>
#include <string>

namespace batchelor {
namespace control {

class Main {
public:
	Main(const Options& options);

	void sendEvent();
	void waitTask(const std::string& taskId);
	void signalTask();
	void showTask();
	void showTasks();

	int getReturnCode() const;

private:
	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection();
	esl::com::http::client::ConnectionFactory& getHTTPConnectionFactory();

	const Options& options;
	std::string url;
	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
	int rc = 0;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_MAIN_H_ */
