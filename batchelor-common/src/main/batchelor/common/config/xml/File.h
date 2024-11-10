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

#ifndef BATCHELOR_COMMON_CONFIG_XML_FILE_H_
#define BATCHELOR_COMMON_CONFIG_XML_FILE_H_

#include <batchelor/common/config/xml/ParseElements.h>
#include <batchelor/common/config/xml/Element.h>

#include <tinyxml2/tinyxml2.h>

#include <memory>
#include <string>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

class File : public ParseElements {
public:

protected:
	File(const std::string& filename, const std::string& rootElementName = "batchelor");

	void parseUserData(void*) override;
	void parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) override;

private:
	struct Document {
		Document(const std::string& filename, const std::string& rootElementName);

		Element getElement() const;

		std::string filename;
		std::unique_ptr<tinyxml2::XMLDocument> xmlDocument;
	};

	std::unique_ptr<tinyxml2::XMLDocument> xmlDocument;
	File(Document document);
};

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_XML_FILE_H_ */
