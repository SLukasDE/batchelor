#ifndef BATCHELOR_SERVICE_SCHEMAS_FETCHRESPONSE_H_
#define BATCHELOR_SERVICE_SCHEMAS_FETCHRESPONSE_H_

#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/Signal.h>

#include "sergut/Util.h"

#include <vector>

namespace batchelor {
namespace service {
namespace schemas {

struct FetchResponse {
	std::vector<Signal> signals;

	// only 0 or 1 runConfigurations.
	std::vector<RunConfiguration> runConfigurations;
};

SERGUT_FUNCTION(FetchResponse, data, ar) {
    ar & SERGUT_NESTED_MMEMBER(data, signals, signals)
       & SERGUT_NESTED_MMEMBER(data, runConfigurations, runConfigurations);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_FETCHRESPONSE_H_ */
