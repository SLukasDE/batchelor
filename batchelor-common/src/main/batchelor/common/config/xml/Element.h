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

#ifndef BATCHELOR_COMMON_CONFIG_XML_ELEMENT_H_
#define BATCHELOR_COMMON_CONFIG_XML_ELEMENT_H_

#include <tinyxml2/tinyxml2.h>

#include <string>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

class Element {
public:
	Element(const tinyxml2::XMLElement& element);

	std::size_t getElementLineNo() const noexcept;
	std::string getElementName() const noexcept;
	const tinyxml2::XMLElement& getXMLElement() const noexcept;

	//static std::size_t toLineNo(const tinyxml2::XMLNode& node) noexcept;
	static std::string toString(tinyxml2::XMLError xmlError);

private:
	const tinyxml2::XMLElement& element;
};

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_XML_ELEMENT_H_ */
