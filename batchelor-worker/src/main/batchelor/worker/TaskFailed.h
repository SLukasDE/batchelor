#ifndef BATCHELOR_WORKER_TASKFAILED_H_
#define BATCHELOR_WORKER_TASKFAILED_H_

#include <batchelor/worker/TaskStatus.h>
#include <batchelor/worker/Task.h>

#include <esl/utility/Signal.h>

namespace batchelor {
namespace worker {

class TaskFailed : public Task {
public:
	TaskFailed(TaskStatus taskStatus);

	TaskStatus getTaskStatus() const override;
	void sendSignal(const esl::utility::Signal& signal) override;

private:
	TaskStatus taskStatus;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKFAILED_H_ */
