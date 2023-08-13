#ifndef BATCHELOR_COMMON_TYPES_STATE_H_
#define BATCHELOR_COMMON_TYPES_STATE_H_

#include <string>

namespace batchelor {
namespace common {
namespace types {

class State final {
public:
	State() = delete;

	enum Type {
		queued, // waiting
		running,
		zombie, // waiting-timeout, running-timeout
		done,
		signaled // failed
	};

	static const std::string& toString(const Type& state);
	static Type toState(const std::string& state);
};

} /* namespace types */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_TYPES_STATE_H_ */
