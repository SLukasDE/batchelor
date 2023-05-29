#ifndef BATCHELOR_WORKER_PROCESSZOMBIE_H_
#define BATCHELOR_WORKER_PROCESSZOMBIE_H_

#include <batchelor/worker/Process.h>

#include <batchelor/common/types/State.h>

#include <esl/utility/Signal.h>

#include <string>

namespace batchelor {
namespace worker {

class ProcessZombie : public Process {
public:

	common::types::State::Type getState() const override;
	int getRC() const override;
	std::string getMessage() const override;
	void sendSignal(const esl::utility::Signal& signal) override;
};

} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PROCESSZOMBIE_H_ */
