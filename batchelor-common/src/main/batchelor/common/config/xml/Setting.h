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

#ifndef BATCHELOR_COMMON_CONFIG_XML_SETTING_H_
#define BATCHELOR_COMMON_CONFIG_XML_SETTING_H_

#include <batchelor/common/config/xml/ParseElements.h>
#include <batchelor/common/config/xml/Element.h>

#include <map>
#include <string>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

struct Setting : public ParseElements {
	static bool stringToBool(bool& b, std::string str);
	static std::string generalEvaluate(const std::string& expression, bool relaxedSyntax, const std::map<std::string, std::string>& keyValues);
	std::string evaluate(const std::string& expression, const std::string& language) const;

	std::string key;
	std::string value;
	std::string language;

protected:
	Setting(const std::string& filename, const Element& element, bool allowLanguage = true);

	void parse();

	void parseUserData(void*) override;
	void parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) override;
	void parseInnerElement(const Element& element) override;

private:
	bool allowLanguage;
	bool hasValue = false;
	bool hasLanguage = false;
};

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_CONFIG_XML_SETTING_H_ */
