#ifndef BATCHELOR_SERVICE_SCHEMAS_SIGNAL_H_
#define BATCHELOR_SERVICE_SCHEMAS_SIGNAL_H_

#include "sergut/Util.h"

#include <string>

namespace batchelor {
namespace service {
namespace schemas {

struct Signal {
	std::string jobId;
	std::string signal;
};

SERGUT_FUNCTION(Signal, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, signal);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_SIGNAL_H_ */
