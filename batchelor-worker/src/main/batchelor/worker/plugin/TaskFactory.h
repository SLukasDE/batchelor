#ifndef BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_
#define BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/Task.h>

#include <esl/object/Object.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
namespace plugin {

class TaskFactory : public esl::object::Object {
public:
	virtual ~TaskFactory() = default;

	virtual bool isBusy() = 0;
	virtual std::unique_ptr<Task> createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const service::schemas::RunConfiguration& runConfiguration) = 0;
};

} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_ */
