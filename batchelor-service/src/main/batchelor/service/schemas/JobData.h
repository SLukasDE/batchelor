#ifndef BATCHELOR_SERVICE_SCHEMAS_JOBDATA_H_
#define BATCHELOR_SERVICE_SCHEMAS_JOBDATA_H_

#include "sergut/Util.h"

#include <string>
#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct JobData {
	std::string jobId;
	std::string node;
	std::string path;
	std::string definition;
//	std::vector<Variable> values;
};

SERGUT_FUNCTION(JobData, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, node)
       & SERGUT_MMEMBER(data, path)
       & SERGUT_MMEMBER(data, definition);
//       & SERGUT_NESTED_MMEMBER(data, values, values);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_JOBDATA_H_ */
