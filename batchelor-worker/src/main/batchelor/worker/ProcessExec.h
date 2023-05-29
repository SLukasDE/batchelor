#ifndef BATCHELOR_WORKER_PROCESSEXEC_H_
#define BATCHELOR_WORKER_PROCESSEXEC_H_

#include <batchelor/worker/Procedure.h>
#include <batchelor/worker/Process.h>
#include <batchelor/worker/ProcessExecFactory.h>

#include <esl/system/Arguments.h>
#include <esl/system/Process.h>
#include <esl/utility/Signal.h>

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class ProcessExec : public Process {
public:
	ProcessExec(ProcessExecFactory& processExecFactory, Procedure& procedure, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& executable, const std::string& workingDirectory);
	~ProcessExec();

	common::types::State::Type getState() const override;
	int getRC() const override;
	std::string getMessage() const override;

	void sendSignal(const esl::utility::Signal& signal) override;

private:
	ProcessExecFactory& processExecFactory;
	Procedure& procedure;
	std::string workingDirectory;

	std::unique_ptr<esl::system::Process> process;
	esl::system::Arguments arguments;
	std::thread thread;
	common::types::State::Type state = common::types::State::Type::queued;
	int returnCode = 0;
	std::string message;

	void run();
};


} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCESSEXEC_H_ */
