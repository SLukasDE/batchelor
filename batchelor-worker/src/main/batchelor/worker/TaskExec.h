#ifndef BATCHELOR_WORKER_TASKEXEC_H_
#define BATCHELOR_WORKER_TASKEXEC_H_

#include <batchelor/worker/Task.h>
#include <batchelor/worker/TaskFactoryExec.h>
#include <batchelor/worker/TaskStatus.h>

#include <esl/system/Arguments.h>
#include <esl/system/Process.h>
#include <esl/utility/Signal.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class TaskExec : public Task {
public:
	TaskExec(TaskFactoryExec& taskFactoryExec, std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& executable, const std::string& workingDirectory);
	~TaskExec();

	TaskStatus getTaskStatus() const override;

	void sendSignal(const esl::utility::Signal& signal) override;

private:
	TaskFactoryExec& taskFactoryExec;

	std::condition_variable& notifyCV;
	std::mutex& taskStatusMutex;
	TaskStatus taskStatus;

	std::string executable;
	std::string workingDirectory;

	std::unique_ptr<esl::system::Process> process;
	esl::system::Arguments arguments;

	std::thread thread;

	void run();
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKEXEC_H_ */
