#ifndef BATCHELOR_SERVICE_SCHEMAS_JOB_H_
#define BATCHELOR_SERVICE_SCHEMAS_JOB_H_

#include <batchelor/service/schemas/Setting.h>

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct Job {
	std::string jobId;
	std::string procedureId;
	std::vector<Setting> settings;
};

SERGUT_FUNCTION(Job, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, procedureId)
       & SERGUT_NESTED_MMEMBER(data, settings, settings);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_JOB_H_ */
