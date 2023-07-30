#ifndef BATCHELOR_CONTROL_ARGUMENTSEXCEPTION_H_
#define BATCHELOR_CONTROL_ARGUMENTSEXCEPTION_H_

#include <stdexcept>

namespace batchelor {
namespace control {

class ArgumentsException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
	//explicit ArgumentsException(database::Diagnostics diagnostics, short int sqlReturnCode);
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_ARGUMENTSEXCEPTION_H_ */
