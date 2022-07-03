#ifndef BATCHELOR_SERVICE_SCHEMAS_STATUS_H_
#define BATCHELOR_SERVICE_SCHEMAS_STATUS_H_

#include "sergut/Util.h"

#include <string>

namespace batchelor {
namespace service {
namespace schemas {

struct Status {
	//std::string jobId;
	std::string state;
};

SERGUT_FUNCTION(Status, data, ar) {
    ar //& SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, state);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_STATUS_H_ */
