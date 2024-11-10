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

#include <batchelor/common/config/xml/FilePosition.h>
#include <batchelor/common/config/xml/Element.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

FilePosition::FilePosition(std::string aFilename, std::size_t aLineNo)
: filename(std::move(aFilename)),
  lineNo(aLineNo)
{ }

const std::string& FilePosition::getFilename() const noexcept {
	return filename;
}
/*
void FilePosition::setLineNo(std::size_t aLineNo) noexcept {
	lineNo = aLineNo;
}
*/
void FilePosition::setLineNo(const Element& element) noexcept {
	lineNo = element.getElementLineNo();
}

std::size_t FilePosition::getLineNo() const noexcept {
	return lineNo;
}

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
