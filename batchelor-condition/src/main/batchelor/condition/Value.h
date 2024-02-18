#ifndef BATCHELOR_CONDITION_VALUE_H_
#define BATCHELOR_CONDITION_VALUE_H_

#include <batchelor/condition/FunctionType.h>
#include <batchelor/condition/ValueType.h>

#include <string>
#include <vector>

namespace batchelor {
namespace condition {

struct Value {
	ValueType getValueType() const;
	//ValueType valueType = ValueType::vtVoid;
	ValueType objectType = ValueType::vtVoid;

	struct {
		std::string name;
		std::vector<Value> args;
		FunctionType function;
	} valueFunction;

	struct {
		std::string name;
	} valueVariable;

    double valueNumber = 0;
    std::string valueString;
    bool valueBool = false;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_VALUE_H_ */
