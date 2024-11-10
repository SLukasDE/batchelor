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
	std::string taskId;
	std::string eventType;

	/* contains event type specific settings, like
	 * - arguments,
	 * - environment variables,
	 * - working dir,
	 * - executable, ...
	 * e.g.:
	 * - settings[0] = { 'args' ; '--propertyId=Bla --propertyFile=/etc/secret/test.pwd' }
	 * - settings[1] = { 'env' ; 'DISPLAY=0' }
	 * - settings[2] = { 'env' ; 'TMP_DIR=/tmp' }
	 * - settings[3] = { 'cd' ; '/var/log' }
	 * - settings[4] = { 'cmd' ; '/opt/bin/true' }
	 */
	std::vector<Setting> settings;

	std::vector<Setting> metrics;
};

SERGUT_FUNCTION(RunConfiguration, data, ar) {
    ar & SERGUT_MMEMBER(data, taskId)
       & SERGUT_MMEMBER(data, eventType)
       & SERGUT_NESTED_MMEMBER(data, settings, settings)
       & SERGUT_NESTED_MMEMBER(data, metrics, metrics);
}

} /* namespace schemas */
} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_SCHEMAS_RUNCONFIGURATION_H_ */
