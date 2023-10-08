/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#ifndef BATCHELOR_COMMON_CRC32_H_
#define BATCHELOR_COMMON_CRC32_H_

#include <cstdint>
#include <string>


namespace batchelor {
namespace common {

class CRC32 {
public:
	CRC32(std::uint32_t initialValue = 0);

	CRC32& pushData(const void* data, std::size_t length);

	std::uint32_t get() const noexcept;

private:
	std::uint32_t value;
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CRC32_H_ */
