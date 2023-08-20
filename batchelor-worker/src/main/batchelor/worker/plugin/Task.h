#ifndef BATCHELOR_WORKER_PLUGIN_TASK_H_
#define BATCHELOR_WORKER_PLUGIN_TASK_H_

#include <batchelor/common/types/State.h>


#include <string>

namespace batchelor {
namespace worker {
namespace plugin {

class Task {
public:
	struct Status {
		common::types::State::Type state;
		int returnCode;
		std::string message; // e.g. exception message
	};

	virtual ~Task() = default;

	virtual Status getStatus() const = 0;
	virtual void sendSignal(const std::string& signal) = 0;
};

} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_TASK_H_ */
