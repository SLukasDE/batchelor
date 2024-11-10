/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

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
