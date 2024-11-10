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

#ifndef BATCHELOR_COMMON_TIMESTAMP_H_
#define BATCHELOR_COMMON_TIMESTAMP_H_

#include <chrono>
#include <string>

namespace batchelor {
namespace common {

class Timestamp final {
public:
	Timestamp() = delete;

	static std::string toJSON(const std::chrono::time_point<std::chrono::system_clock>& time_point);
	static std::chrono::time_point<std::chrono::system_clock> fromJSON(const std::string& str);

	static std::string toString(const std::chrono::time_point<std::chrono::system_clock>& time_point);
	static std::chrono::time_point<std::chrono::system_clock> fromString(const std::string& str);

	static std::chrono::milliseconds toDuration(const std::string& str);
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_TIMESTAMP_H_ */
