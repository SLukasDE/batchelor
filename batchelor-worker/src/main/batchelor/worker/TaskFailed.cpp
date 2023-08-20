#include <batchelor/worker/TaskFailed.h>

#include <memory>

namespace batchelor {
namespace worker {

TaskFailed::TaskFailed(plugin::Task::Status aStatus)
: status(std::move(aStatus))
{ }

plugin::Task::Status TaskFailed::getStatus() const {
	return status;
}

void TaskFailed::sendSignal(const std::string& signal) {
}

} /* namespace worker */
} /* namespace batchelor */
