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

#include <batchelor/common/config/xml/Setting.h>

#include <tinyxml2.h>

#include <esl/io/FilePosition.h>

#include <unistd.h>
extern char **environ;

#include <stdexcept>

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

std::string Setting::generalEvaluate(const std::string& expression, bool relaxedSyntax, const std::map<std::string, std::string>& keyValues) {
	std::string value;
	std::string varName;
	std::string varString;
	enum {
		intro,
		begin,
		end
	} state = end;

	for(std::size_t i=0; i<expression.size(); ++i) {
		if(state == begin) {
			varString += expression.at(i);
			if(expression.at(i) == '}') {
				auto iter = keyValues.find(varName);
				if(iter != keyValues.end()) {
					value += iter->second;
				}
				else if(relaxedSyntax) {
					value += varString;
				}
				else {
					throw std::runtime_error("No value available for variable \"" + varName + "\" in expression: \"" + expression + "\"");
				}

				state = end;
				varString.clear();
				varName.clear();
			}
			else {
				varName += expression.at(i);
			}
		}
		else if(state == intro) {
			varString += expression.at(i);
			if(expression.at(i) == '{') {
				state = begin;
			}
			else if(relaxedSyntax) {
				value += varString;
				varString.clear();
			}
			else {
				throw std::runtime_error("Syntax error in expression: \"" + expression + "\"");
			}
		}
		else {
			if(expression.at(i) == '$') {
				state = intro;
				varName.clear();
				varString = expression.at(i);
			}
			else {
				value += expression.at(i);
			}
		}
	}

	return value;
}

std::string Setting::evaluate(const std::string& expression, const std::string& language) const {
	if(language == "plain") {
		return expression;
	}

	try {
		std::map<std::string, std::string> keyValues;

		for(char **s = environ; *s; s++) {
			std::string env(*s);
			std::size_t pos = env.find('=');
			std::string key = env.substr(0, pos);
			std::string value;
			if(pos != std::string::npos) {
		        value = env.substr(pos+1);
			}
			keyValues[key] = value;
		}

		return generalEvaluate(expression, language.empty(), keyValues);
	}
	catch(const std::exception& e) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error(e.what()));
	}
	catch(...) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("General error in expression: \"" + expression + "\""));
	}
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
