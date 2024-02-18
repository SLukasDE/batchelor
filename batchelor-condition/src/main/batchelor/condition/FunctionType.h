#ifndef BATCHELOR_CONDITION_FUNCTIONTYPE_H_
#define BATCHELOR_CONDITION_FUNCTIONTYPE_H_

#include <batchelor/condition/ValueType.h>

#include <string>
#include <vector>

namespace batchelor {
namespace condition {

struct FunctionType {
	ValueType returnType;
	std::vector<ValueType> arguments;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_FUNCTIONTYPE_H_ */
