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

#ifndef BATCHELOR_COMMON_CONFIG_ARGS_ARGUMENTSEXCEPTION_H_
#define BATCHELOR_COMMON_CONFIG_ARGS_ARGUMENTSEXCEPTION_H_

#include <stdexcept>

namespace batchelor {
namespace common {
namespace config {
namespace args {

class ArgumentsException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
	//explicit ArgumentsException(database::Diagnostics diagnostics, short int sqlReturnCode);
};

} /* namespace args */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_ARGS_ARGUMENTSEXCEPTION_H_ */
