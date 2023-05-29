#ifndef BATCHELOR_WORKER_PROCESSEXECFACTORY_H_
#define BATCHELOR_WORKER_PROCESSEXECFACTORY_H_

#include <batchelor/worker/Process.h>
#include <batchelor/worker/ProcessFactory.h>

#include <esl/object/Object.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class ProcessExecFactory : public ProcessFactory {
public:
	struct Settings {
		std::string executable;
		std::string workingDirectory;
		std::size_t maximumJobsRunning = 0;
	};

	ProcessExecFactory(Settings settings);

	static std::unique_ptr<ProcessFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);
	static std::unique_ptr<esl::object::Object> createObject(const std::vector<std::pair<std::string, std::string>>& settings);

	bool isBusy(Procedure& procedure) override;
	std::unique_ptr<Process> createProcess(Procedure& procedure, const std::vector<std::pair<std::string, std::string>>& settings) override;

	void releaseProcess();

private:
	Settings settings;
	std::size_t processesRunning = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCESSEXECFACTORY_H_ */
