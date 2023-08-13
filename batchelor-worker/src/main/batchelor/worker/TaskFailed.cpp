#include <batchelor/worker/TaskFailed.h>

#include <memory>

namespace batchelor {
namespace worker {

TaskFailed::TaskFailed(TaskStatus aTaskStatus)
: taskStatus(std::move(aTaskStatus))
{ }

TaskStatus TaskFailed::getTaskStatus() const {
	return taskStatus;
}

void TaskFailed::sendSignal(const esl::utility::Signal& signal) {
}

} /* namespace worker */
} /* namespace batchelor */
