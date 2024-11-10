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

#include <batchelor/condition/Value.h>

namespace batchelor {
namespace condition {

Value::Value(bool b)
: objectType(ObjectType::otBool),
  valueBool(b)
{ }

Value::Value(double number)
: objectType(ObjectType::otNumber),
  valueNumber(number)
{ }

Value::Value(const std::string& str)
: objectType(ObjectType::otString),
  valueString(str)
{ }

ValueType Value::getValueType() const {
	switch(objectType) {
	case otFunction:
		return functionType.returnType;
	case otVariable:
		return ValueType::vtString;
	case otNumber:
		return ValueType::vtNumber;
	case otString:
		return ValueType::vtString;
	case otBool:
	default:
		return ValueType::vtBool;
	}
}

} /* namespace condition */
} /* namespace batchelor */
