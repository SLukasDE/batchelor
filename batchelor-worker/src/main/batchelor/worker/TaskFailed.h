#ifndef BATCHELOR_WORKER_TASKFAILED_H_
#define BATCHELOR_WORKER_TASKFAILED_H_

#include <batchelor/worker/plugin/Task.h>

#include <string>

namespace batchelor {
namespace worker {

class TaskFailed : public plugin::Task {
public:

	TaskFailed(plugin::Task::Status status);

	plugin::Task::Status getStatus() const override;
	void sendSignal(const std::string& signal) override;

private:
	plugin::Task::Status status;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKFAILED_H_ */
