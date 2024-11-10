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

#ifndef BATCHELOR_SERVICE_LOGGER_H_
#define BATCHELOR_SERVICE_LOGGER_H_

#include <esl/monitoring/Logger.h>
#include <esl/monitoring/Streams.h>

namespace batchelor {
namespace service {

using Logger = esl::monitoring::Logger<esl::monitoring::Streams::Level::TRACE>;

} /* namespace service */
} /* namespace batchelor */

#endif /* BATCHELOR_SERVICE_LOGGER_H_ */
