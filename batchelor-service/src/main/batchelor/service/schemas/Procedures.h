#ifndef BATCHELOR_SERVICE_SCHEMAS_PROCEDURES_H_
#define BATCHELOR_SERVICE_SCHEMAS_PROCEDURES_H_

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct Procedures {
	std::vector<std::string> ids;
};

SERGUT_FUNCTION(Procedures, data, ar) {
    ar & SERGUT_NESTED_MMEMBER(data, ids, ids);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_PROCEDURES_H_ */
