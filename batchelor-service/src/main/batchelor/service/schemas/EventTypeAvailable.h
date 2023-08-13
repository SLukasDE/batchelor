#ifndef BATCHELOR_SERVICE_SCHEMAS_EVENTTYPEAVAILABLE_H_
#define BATCHELOR_SERVICE_SCHEMAS_EVENTTYPEAVAILABLE_H_

#include "sergut/Util.h"

#include <string>

namespace batchelor {
namespace service {
namespace schemas {

struct EventTypeAvailable {
	std::string eventType;

	/* this flag is true if event type is available to create a new task for it.
	 * Flag is false if there is no more capacity to create a new task.
	 */
	bool available;
};

SERGUT_FUNCTION(EventTypeAvailable, data, ar) {
    ar & SERGUT_MMEMBER(data, eventType)
       & SERGUT_MMEMBER(data, available);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_EVENTTYPEAVAILABLE_H_ */
