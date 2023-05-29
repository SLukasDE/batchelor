#include <batchelor/worker/ProcessZombie.h>

namespace batchelor {
namespace worker {

common::types::State::Type ProcessZombie::getState() const {
	return common::types::State::Type::zombie;
}

int ProcessZombie::getRC() const {
	return -1;
}

std::string ProcessZombie::getMessage() const {
	return "worker restarted";
}

void ProcessZombie::sendSignal(const esl::utility::Signal&) {
}

} /* namespace worker */
} /* namespace batchelor */
