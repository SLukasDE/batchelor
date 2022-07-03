#ifndef BATCHELOR_SERVICE_SCHEMAS_SETTING_H_
#define BATCHELOR_SERVICE_SCHEMAS_SETTING_H_

#include "sergut/Util.h"

#include <string>

namespace batchelor {
namespace service {
namespace schemas {

struct Setting {
	std::string key;
	std::string value;
};

SERGUT_FUNCTION(Setting, data, ar) {
    ar & SERGUT_MMEMBER(data, key)
       & SERGUT_MMEMBER(data, value);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_SETTING_H_ */
