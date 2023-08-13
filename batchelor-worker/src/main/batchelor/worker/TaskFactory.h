#ifndef BATCHELOR_WORKER_TASKFACTORY_H_
#define BATCHELOR_WORKER_TASKFACTORY_H_

#include <batchelor/worker/Task.h>

#include <esl/object/Object.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {

class Main;

class TaskFactory : public esl::object::Object {
public:
	virtual ~TaskFactory() = default;

	virtual bool isBusy() = 0;
	virtual std::unique_ptr<Task> createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& settings) = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKFACTORY_H_ */
