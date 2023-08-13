#ifndef BATCHELOR_WORKER_TASK_H_
#define BATCHELOR_WORKER_TASK_H_

#include <batchelor/worker/TaskStatus.h>

#include <esl/utility/Signal.h>

namespace batchelor {
namespace worker {

class Task {
public:
	virtual ~Task() = default;

	virtual TaskStatus getTaskStatus() const = 0;
	virtual void sendSignal(const esl::utility::Signal& signal) = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASK_H_ */
