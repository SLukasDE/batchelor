#ifndef BATCHELOR_CONDITION_COBJECT_H_
#define BATCHELOR_CONDITION_COBJECT_H_

#include <memory>
#include <string>
#include <vector>

namespace batchelor {
namespace condition {

struct CObject {
	CObject();

	enum Type {
		otProcedure, otVoid, otDouble, otString, otBool
	};

	std::string name;
	Type type = otVoid;
    std::string v_string;
    double v_double = 0;
    bool v_bool = false;

    void add(std::vector<CObject>& aObjectList);

    std::unique_ptr<std::vector<CObject>> objectList;
};

using CObjectList = std::vector<CObject>;

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_COBJECT_H_ */
