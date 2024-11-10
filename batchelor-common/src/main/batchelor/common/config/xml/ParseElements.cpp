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

#include <batchelor/common/config/xml/ParseElements.h>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

ParseElements::ParseElements(std::string filename, const Element& aElement)
: FilePosition(std::move(filename), aElement.getElementLineNo()),
  element(aElement)
{ }

void ParseElements::parse() {
	if(element.getXMLElement().GetUserData() != nullptr) {
		parseUserData(element.getXMLElement().GetUserData());
	}

	for(const tinyxml2::XMLAttribute* attribute = element.getXMLElement().FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		parseInnerAttribute(*attribute);
	}

	for(const tinyxml2::XMLNode* node = element.getXMLElement().FirstChild(); node != nullptr; node = node->NextSibling()) {
//		setLineNo(Element::toLineNo(*node));

		const tinyxml2::XMLElement* innerElement = node->ToElement();
		if(innerElement == nullptr) {
			continue;
		}
		Element element(*innerElement);

		setLineNo(element);
		parseInnerElement(element);
	}
}

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
