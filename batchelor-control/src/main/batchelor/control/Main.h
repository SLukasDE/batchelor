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
	void waitTask();
	void cancelTask();
	void signalTask();
	void showTask();
	void showTasks();

	std::unique_ptr<esl::com::http::client::Connection> createConnection();
	esl::com::http::client::ConnectionFactory& getConnectionFactory();

private:
	const Options& options;
	std::string url;
	std::unique_ptr<esl::com::http::client::ConnectionFactory> httpConnectionFactory;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_MAIN_H_ */
