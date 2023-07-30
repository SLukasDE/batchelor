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
	std::string eventType;

	/* contains event type specific settings, like
	 * - arguments,
	 * - environment variables,
	 * - working dir, ...
	 * e.g.:
	 * - settings[0] = { 'env' ; 'DISPLAY=0' }
	 * - settings[1] = { 'env' ; 'TMP_DIR=/tmp' }
	 * - settings[2] = { 'args' ; '--propertyId=Bla --propertyFile=/wxx/secret/property.cfg' }
	 */
	std::vector<Setting> settings;
};

SERGUT_FUNCTION(RunConfiguration, data, ar) {
    ar & SERGUT_MMEMBER(data, jobId)
       & SERGUT_MMEMBER(data, eventType)
       & SERGUT_NESTED_MMEMBER(data, settings, settings);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNCONFIGURATION_H_ */
