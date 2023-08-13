#ifndef BATCHELOR_SERVICE_SCHEMAS_RUNRESPONSE_H_
#define BATCHELOR_SERVICE_SCHEMAS_RUNRESPONSE_H_

#include "sergut/Util.h"

#include <string>

namespace batchelor {
namespace service {
namespace schemas {

struct RunResponse {
	std::string taskId;
	std::string message;
};

SERGUT_FUNCTION(RunResponse, data, ar) {
    ar & SERGUT_MMEMBER(data, taskId)
       & SERGUT_MMEMBER(data, message);
}

} /* namespace schema */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNRESPONSE_H_ */
