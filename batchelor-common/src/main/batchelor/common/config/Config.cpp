#include <batchelor/common/config/Config.h>
#include <batchelor/common/config/FilePosition.h>
//#include <batchelor/Logger.h>

#include <cstdlib>

namespace batchelor {
namespace common {
namespace config {

namespace {
//Logger logger("jerry4esl::config::Config");
} /* anonymous namespace */

Config::Config(std::string aFilename)
: filename(std::move(aFilename))
{ }

Config::Config(std::string aFilename, std::size_t aLineNo)
: filename(std::move(aFilename)),
  lineNo(aLineNo)
{ }

std::string Config::toString(tinyxml2::XMLError xmlError) {
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

std::string Config::evaluate(const std::string& expression, const std::string& language) const {
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
					throw FilePosition::add(*this, "No value available for variable \"" + var + "\" in expression: \"" + expression + "\"");
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
				throw FilePosition::add(*this, "Syntax error in expression: \"" + expression + "\"");
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

void Config::setFilename(std::string aFilename) {
	filename = std::move(aFilename);
}

const std::string& Config::getFilename() const noexcept {
	return filename;
}

void Config::setLineNo(std::size_t aLineNo) {
	lineNo = aLineNo;
}

void Config::setLineNo(const tinyxml2::XMLElement& aElement) {
	lineNo = static_cast<std::size_t>(aElement.GetLineNum() < 0 ? 0 : aElement.GetLineNum());
}

std::size_t Config::getLineNo() const noexcept {
	return lineNo;
}
/*
std::pair<std::string, int> Config::getXMLFile() const noexcept {
	return std::pair<std::string, int>(filename, lineNo);
}

std::pair<std::string, int> Config::setXMLFile(const std::string& aFilename, int aLineNo) {
	std::pair<std::string, int> oldXmlFile(filename, lineNo);

	filename = aFilename;
	lineNo = aLineNo;

	return oldXmlFile;
}

std::pair<std::string, int> Config::setXMLFile(const std::string& aFilename, const tinyxml2::XMLElement& aElement) {
	std::pair<std::string, int> oldXmlFile(filename, lineNo);

	filename = aFilename;
	lineNo = aElement.GetLineNum();

	return oldXmlFile;
}

std::pair<std::string, int> Config::setXMLFile(const std::pair<std::string, int>& aXmlFile) {
	std::pair<std::string, int> oldXmlFile(filename, lineNo);

	filename = aXmlFile.first;
	lineNo = aXmlFile.second;

	return oldXmlFile;
}
*/
bool Config::stringToBool(bool& b, std::string str) {
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

} /* namespace config */
} /* namespace common */
} /* namespace batchelor */
