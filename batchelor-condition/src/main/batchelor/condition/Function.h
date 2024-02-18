#ifndef BATCHELOR_CONDITION_FUNCTION_H_
#define BATCHELOR_CONDITION_FUNCTION_H_

#include <batchelor/condition/ValueType.h>

#include <string>
#include <vector>

namespace batchelor {
namespace condition {

struct Function {
	std::string name;

	ValueType returnType;
	std::vector<ValueType> arguments;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_FUNCTION_H_ */
