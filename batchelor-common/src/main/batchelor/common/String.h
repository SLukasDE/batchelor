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

#ifndef BATCHELOR_COMMON_STRING_H_
#define BATCHELOR_COMMON_STRING_H_

#include <esl/system/Stacktrace.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace batchelor {
namespace common {

class String final {
public:
	String() = delete;

	template<typename OType>
	static OType toNumber(const std::string& str) {
		return checkedNumericConvert<OType>(std::stoll(str));
	}

private:
	template<typename OType, typename IType>
	static OType checkedNumericConvert(IType inputValue) {
	    if(std::is_signed<IType>::value && std::is_unsigned<OType>::value && inputValue < 0) {
	        throw esl::system::Stacktrace::add(std::runtime_error("Converting negative number to unsigned integer type"));
	    }
	    if(std::is_signed<IType>::value && std::is_signed<OType>::value && inputValue < std::numeric_limits<OType>::lowest()) {
	        throw esl::system::Stacktrace::add(std::runtime_error("Numeric underflow in conversion"));
	    }
	    if(inputValue > std::numeric_limits<OType>::max()) {
	        throw esl::system::Stacktrace::add(std::runtime_error("Numeric overflow in conversion"));
	    }
	    return static_cast<OType>(inputValue);
	}
};

} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_STRING_H_ */
