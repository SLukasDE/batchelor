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
