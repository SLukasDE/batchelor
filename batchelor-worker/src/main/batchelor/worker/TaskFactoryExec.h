#ifndef BATCHELOR_WORKER_TASKFACTORYEXEC_H_
#define BATCHELOR_WORKER_TASKFACTORYEXEC_H_

#include <batchelor/worker/Task.h>
#include <batchelor/worker/TaskFactory.h>

#include <esl/object/Object.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class TaskFactoryExec : public TaskFactory {
public:
	struct Settings {
		std::string executable;
		std::string workingDirectory;
		std::size_t maximumJobsRunning = 0;
	};

	TaskFactoryExec(Settings settings);

	/* settings:
	 * - arguments and
	 * - task specific arguments-flag, tells if task specific arguments are
	 *   - overwriting these arguments or
	 *   - extending these arguments or
	 *   - not allowed
	 * - environment variables and
	 * - global environment-flag, tells if these environments are
	 *   - overwriting system environments or
	 *   - extending system environments and
	 * - task specific environment-flag, tells if these environments are
	 *   - overwriting these environments or
	 *   - extending these environments or
	 *   - not allowed
	 * - working dir,
	 * - executable, ...
	 * e.g.:
	 * - settings[ 0] = { 'max-tasks-running' ; '3' }
	 * - settings[ 1] = { 'args' ;              '--propertyId=Bla --propertyFile=/wxx/secret/property.cfg' }
	 * - settings[ 2] = { 'args-flag' ;         'override|extend|fixed' }
	 * - settings[ 3] = { 'env' ;               'DISPLAY=0' }
	 * - settings[ 4] = { 'env' ;               'TMP_DIR=/tmp' }
	 * - settings[ 5] = { 'env-flag-global' ;   'override|extend' }
	 * - settings[ 6] = { 'env-flag' ;          'override|extend|fixed' }
	 * - settings[ 7] = { 'cd' ;                '/wxx/app/rose/log' }
	 * - settings[ 8] = { 'cd-flag' ;           'override|fixed' }
	 * - settings[ 9] = { 'cmd' ;               '/opt/bin/bestoptxl-calculation' }
	 * - settings[10] = { 'cmd-flag' ;          'override|fixed' }
	 */
	static std::unique_ptr<TaskFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	bool isBusy() override;
	/* settings:
	 * - arguments,
	 * - environment variables,
	 * - working dir,
	 * - executable, ...
	 * e.g.:
	 * - settings[0] = { 'args' ; '--propertyId=Bla --propertyFile=/wxx/secret/property.cfg' }
	 * - settings[1] = { 'env'  ; 'DISPLAY=0' }
	 * - settings[2] = { 'env' ;  'TMP_DIR=/tmp' }
	 * - settings[3] = { 'cd' ;   '/wxx/app/rose/log' }
	 * - settings[4] = { 'cmd' ;  '/opt/bin/bestoptxl-calculation' }
	 */
	std::unique_ptr<Task> createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& settings) override;

	void releaseProcess();

private:
	Settings settings;
	std::size_t processesRunning = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKFACTORYEXEC_H_ */
