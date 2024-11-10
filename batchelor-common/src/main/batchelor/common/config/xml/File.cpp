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
#include <batchelor/common/config/xml/File.h>

#include <esl/io/FilePosition.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

File::Document::Document(const std::string& aFilename, const std::string& rootElementName)
: filename(aFilename),
  xmlDocument(new tinyxml2::XMLDocument)
{
	tinyxml2::XMLError xmlError = xmlDocument->LoadFile(filename.c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw esl::io::FilePosition::add(filename, 0, std::runtime_error(Element::toString(xmlError)));
	}

	const tinyxml2::XMLElement* xmlElement = xmlDocument->RootElement();
	if(xmlElement == nullptr) {
		throw esl::io::FilePosition::add(filename, 0, std::runtime_error("No root element in file \"" + filename + "\""));
	}

	Element element(getElement());
	if(element.getElementName() != rootElementName) {
		throw esl::io::FilePosition::add(filename, element.getElementLineNo(), std::runtime_error("Name of XML root element is \"" + element.getElementName() + "\" but should be \"" + rootElementName + "\""));
	}
}

Element File::Document::getElement() const {
	const tinyxml2::XMLElement* xmlElement = xmlDocument->RootElement();
	if(xmlElement == nullptr) {
		throw esl::io::FilePosition::add(filename, 0, std::runtime_error("No root element in file \"" + filename + "\""));
	}
	return Element(*xmlElement);
}

File::File(const std::string& filename, const std::string& rootElementName)
: File(Document(filename, rootElementName))
{ }

File::File(Document aDocument)
: ParseElements(aDocument.filename, aDocument.getElement()),
  xmlDocument(std::move(aDocument.xmlDocument))
{ }

void File::parseUserData(void*) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
}

void File::parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + std::string(attribute.Name()) + "'"));
}

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
