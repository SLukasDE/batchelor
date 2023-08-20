#ifndef BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_
#define BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/exec/TaskFactory.h>
#include <batchelor/worker/plugin/Task.h>

#include <esl/system/Arguments.h>
#include <esl/system/Process.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
namespace plugin {
namespace exec {

class Task : public plugin::Task {
public:
	struct Settings {
		std::string args;
		std::map<std::string, std::string> envs;
		std::string cd;
		std::string cmd;
	};

	Task(TaskFactory& taskFactoryExec, std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration);
	~Task();

	Status getStatus() const override;

	void sendSignal(const std::string& signal) override;

private:
	TaskFactory& taskFactory;

	std::condition_variable& notifyCV;
	std::mutex& taskStatusMutex;
	Status status;

	Settings settings;

	std::unique_ptr<esl::system::Process> process;
	esl::system::Arguments arguments;

	std::thread thread;

	void run();
};

} /* namespace exec */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_ */
