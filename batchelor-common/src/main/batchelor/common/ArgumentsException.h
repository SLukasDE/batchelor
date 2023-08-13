#ifndef BATCHELOR_COMMON_ARGUMENTSEXCEPTION_H_
#define BATCHELOR_COMMON_ARGUMENTSEXCEPTION_H_

#include <stdexcept>

namespace batchelor {
namespace common {

class ArgumentsException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
	//explicit ArgumentsException(database::Diagnostics diagnostics, short int sqlReturnCode);
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_ARGUMENTSEXCEPTION_H_ */
