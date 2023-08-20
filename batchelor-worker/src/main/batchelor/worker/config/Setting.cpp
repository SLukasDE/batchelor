#include <batchelor/common/config/FilePosition.h>

#include <batchelor/worker/config/Setting.h>

namespace batchelor {
namespace worker {
namespace config {

using common::config::FilePosition;

Setting::Setting(const std::string& filename, const tinyxml2::XMLElement& element, bool allowLanguage)
: common::config::Config(filename, element.GetLineNum())
{
	if(element.GetUserData() != nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
	}

	bool hasValue = false;
	bool hasLanguage = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "key") {
			if(!key.empty()) {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'key'."));
			}
			key = attribute->Value();
			if(key.empty()) {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'key' is invalid."));
			}
		}
		else if(std::string(attribute->Name()) == "value") {
			if(hasValue) {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'value'."));
			}
			value = attribute->Value();
			hasValue = true;
		}
		else if(std::string(attribute->Name()) == "language" && allowLanguage) {
			if(hasLanguage) {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute 'language'."));
			}
			language = attribute->Value();
			hasLanguage = true;
		}
		else {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + std::string(attribute->Name()) + "'"));
		}
	}

	if(key.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'key'"));
	}
	if(!hasValue) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'value'"));
	}
}

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
