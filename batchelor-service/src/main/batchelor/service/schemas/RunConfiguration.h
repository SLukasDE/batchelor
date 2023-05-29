#ifndef BATCHELOR_SERVICE_SCHEMAS_RUNCONFIGURATION_H_
#define BATCHELOR_SERVICE_SCHEMAS_RUNCONFIGURATION_H_

#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct RunConfiguration {
	std::string jobId;
	std::string batchId;
	std::vector<Setting> settings;
};

SERGUT_FUNCTION(RunConfiguration, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, batchId)
       & SERGUT_NESTED_MMEMBER(data, settings, settings);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNCONFIGURATION_H_ */
