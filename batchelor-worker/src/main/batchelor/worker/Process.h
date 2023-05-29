#ifndef BATCHELOR_WORKER_PROCESS_H_
#define BATCHELOR_WORKER_PROCESS_H_

#include <batchelor/common/types/State.h>

#include <esl/utility/Signal.h>

#include <string>

namespace batchelor {
namespace worker {

class Process {
public:
	virtual ~Process() = default;

	virtual common::types::State::Type getState() const = 0;
	virtual int getRC() const = 0;
	virtual std::string getMessage() const = 0;

	virtual void sendSignal(const esl::utility::Signal& signal) = 0;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCESS_H_ */
