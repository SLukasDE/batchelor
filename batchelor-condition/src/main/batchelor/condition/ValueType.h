#ifndef BATCHELOR_CONDITION_VALUETYPE_H_
#define BATCHELOR_CONDITION_VALUETYPE_H_

namespace batchelor {
namespace condition {

enum ValueType {
	vtVoid,
	vtFunction,
	vtVariable,
	vtNumber,
	vtString,
	vtBool
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_VALUETYPE_H_ */
