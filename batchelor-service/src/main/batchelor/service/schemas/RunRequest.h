#ifndef BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_
#define BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_

#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct RunRequest {
	std::string batchId;
	unsigned int priority;
	std::vector<std::string> arguments;
	std::vector<Setting> envVars;
	std::vector<Setting> settings;
};

SERGUT_FUNCTION(RunRequest, data, ar) {
    ar & SERGUT_MMEMBER(data, batchId)
       & SERGUT_MMEMBER(data, priority)
       & SERGUT_NESTED_MMEMBER(data, arguments, args)
       & SERGUT_NESTED_MMEMBER(data, envVars, envs)
       & SERGUT_NESTED_MMEMBER(data, settings, settings);
}

} /* namespace schema */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNREQUEST_H_ */
