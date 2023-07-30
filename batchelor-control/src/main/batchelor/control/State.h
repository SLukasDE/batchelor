#ifndef BATCHELOR_CONTROL_STATE_H_
#define BATCHELOR_CONTROL_STATE_H_

namespace batchelor {
namespace control {

enum class State {
	waiting,
	timeout,
	running,
	done,
	failed,
	zombi
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_STATE_H_ */
