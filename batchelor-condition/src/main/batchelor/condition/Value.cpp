#include <batchelor/condition/Value.h>

namespace batchelor {
namespace condition {

ValueType Value::getValueType() const {
	switch(objectType) {
	case vtFunction:
		return valueFunction.function.returnType;
	case vtVariable:
		return ValueType::vtString;
	default:
		return objectType;
	}
}

} /* namespace condition */
} /* namespace batchelor */
