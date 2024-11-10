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

#ifndef BATCHELOR_COMMON_CONFIG_FILEPOSITION_XML_H_
#define BATCHELOR_COMMON_CONFIG_FILEPOSITION_XML_H_

#include <string>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

class Element;

class FilePosition {
public:
	FilePosition() = delete;
	FilePosition(std::string filename, std::size_t lineNo = 0);
	virtual ~FilePosition() = default;

	const std::string& getFilename() const noexcept;

	//void setLineNo(std::size_t aLineNo) noexcept;
	void setLineNo(const Element& element) noexcept;
	std::size_t getLineNo() const noexcept;

private:
	const std::string filename;
	std::size_t lineNo;
};

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_FILEPOSITION_XML_H_ */
