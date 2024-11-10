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

#include <batchelor/common/config/xml/Element.h>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

Element::Element(const tinyxml2::XMLElement& aElement)
: element(aElement)
{ }

std::size_t Element::getElementLineNo() const noexcept {
	return static_cast<std::size_t>(element.GetLineNum() < 0 ? 0 : element.GetLineNum());
}

std::string Element::getElementName() const noexcept {
	return element.Name() ? element.Name() : "";
}

const tinyxml2::XMLElement& Element::getXMLElement() const noexcept {
	return element;
}
/*
std::size_t Element::toLineNo(const tinyxml2::XMLNode& node) noexcept {
	return static_cast<std::size_t>(node.GetLineNum() < 0 ? 0 : node.GetLineNum());
}
*/
std::string Element::toString(tinyxml2::XMLError xmlError) {
	switch(xmlError) {
	case tinyxml2::XML_SUCCESS:
		return "XML_SUCCESS";
	case tinyxml2::XML_NO_ATTRIBUTE:
		return "XML_NO_ATTRIBUTE";
	case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
		return "XML_WRONG_ATTRIBUTE_TYPE";
	case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
		return "XML_ERROR_FILE_NOT_FOUND";
	case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		return "XML_ERROR_FILE_COULD_NOT_BE_OPENED";
	case tinyxml2::XML_ERROR_FILE_READ_ERROR:
		return "XML_ERROR_FILE_READ_ERROR";
	case tinyxml2::XML_ERROR_PARSING_ELEMENT:
		return "XML_ERROR_PARSING_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE:
		return "XML_ERROR_PARSING_ATTRIBUTE";
	case tinyxml2::XML_ERROR_PARSING_TEXT:
		return "XML_ERROR_PARSING_TEXT";
	case tinyxml2::XML_ERROR_PARSING_CDATA:
		return "XML_ERROR_PARSING_CDATA";
	case tinyxml2::XML_ERROR_PARSING_COMMENT:
		return "XML_ERROR_PARSING_COMMENT";
	case tinyxml2::XML_ERROR_PARSING_DECLARATION:
		return "XML_ERROR_PARSING_DECLARATION";
	case tinyxml2::XML_ERROR_PARSING_UNKNOWN:
		return "XML_ERROR_PARSING_UNKNOWN";
	case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:
		return "XML_ERROR_EMPTY_DOCUMENT";
	case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT:
		return "XML_ERROR_MISMATCHED_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING:
		return "XML_ERROR_PARSING";
	case tinyxml2::XML_CAN_NOT_CONVERT_TEXT:
		return "XML_CAN_NOT_CONVERT_TEXT";
	case tinyxml2::XML_NO_TEXT_NODE:
		return "XML_NO_TEXT_NODE";
	case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED:
		return "XML_ELEMENT_DEPTH_EXCEEDED";
	case tinyxml2::XML_ERROR_COUNT:
		return "XML_ERROR_COUNT";
	default:
		break;
	}

	return "unknown XML error";
}

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
