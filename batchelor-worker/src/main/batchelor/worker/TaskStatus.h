#ifndef BATCHELOR_WORKER_TASKSTATUS_H_
#define BATCHELOR_WORKER_TASKSTATUS_H_

#include <batchelor/common/types/State.h>

#include <string>

namespace batchelor {
namespace worker {

struct TaskStatus {
	common::types::State::Type state;
	int returnCode;
	std::string message; // e.g. exception message
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_TASKSTATUS_H_ */
