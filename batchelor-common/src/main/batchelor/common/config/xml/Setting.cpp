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

#include <batchelor/common/config/xml/Setting.h>

#include <tinyxml2/tinyxml2.h>

#include <esl/io/FilePosition.h>

namespace batchelor {
namespace common {
namespace config {
namespace xml {

Setting::Setting(const std::string& filename, const Element& element, bool aAllowLanguage)
: ParseElements(filename, element),
  allowLanguage(aAllowLanguage)
{ }

void Setting::parse() {
	ParseElements::parse();

	if(key.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'key'"));
	}

	if(!hasValue) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'value'"));
	}
}

bool Setting::stringToBool(bool& b, std::string str) {
	if(str == "true") {
		b = true;
	}
	else if(str == "false") {
		b = false;
	}
	else {
		return false;
	}
	return true;
}

std::string Setting::evaluate(const std::string& expression, const std::string& language) const {
	if(language == "plain") {
		return expression;
	}

	std::string value;
	std::string var;
	enum {
		intro,
		begin,
		end
	} state = end;

	for(std::size_t i=0; i<expression.size(); ++i) {
		if(state == begin) {
			if(expression.at(i) == '}') {
				char* val = getenv(var.c_str());
				if(val == nullptr) {
					throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("No value available for variable \"" + var + "\" in expression: \"" + expression + "\""));
				}
				value += val;
				state = end;
				var.clear();
			}
			else {
				var += expression.at(i);
			}
		}
		else if(state == intro) {
			if(expression.at(i) == '{') {
				state = begin;
			}
			else {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Syntax error in expression: \"" + expression + "\""));
			}
		}
		else {
			if(expression.at(i) == '$') {
				state = intro;
			}
			else {
				value += expression.at(i);
			}
		}
	}

	return value;
}

void Setting::parseUserData(void*) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
}

void Setting::parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) {
	std::string attributeName(attribute.Name());

	if(attributeName == "key") {
		if(!key.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'key'."));
		}
		key = attribute.Value();
		if(key.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'key' is invalid."));
		}
	}
	else if(attributeName == "value") {
		if(hasValue) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'value'."));
		}
		value = attribute.Value();
		hasValue = true;
	}
	else if(attributeName == "language" && allowLanguage) {
		if(hasLanguage) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'language'."));
		}
		language = attribute.Value();
		hasLanguage = true;
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + attributeName + "'"));
	}
}

void Setting::parseInnerElement(const Element& element) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has inner elements but it should be empty"));
}

} /* namespace xml */
} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
