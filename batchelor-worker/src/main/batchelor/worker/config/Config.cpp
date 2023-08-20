#include <batchelor/common/config/FilePosition.h>
#include <batchelor/worker/config/Event.h>

#include <batchelor/worker/config/Config.h>
#include <batchelor/worker/config/Setting.h>

#include <iostream>

namespace batchelor {
namespace worker {
namespace config {

using common::config::FilePosition;

Config::Config(Main& aMain, const std::string& filename)
: common::config::Config(filename),
  main(aMain)
{
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(filename.c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error(Config::toString(xmlError)));
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("No root element in file \"" + filename + "\""));
	}

	setLineNo(element->GetLineNum());

	if(element->Name() == nullptr) {
		throw FilePosition::add(*this, "Name of XML root element is empty");
	}

	const std::string elementName(element->Name());

	if(elementName != "batchelor") {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Name of XML root element is \"" + elementName + "\" but should be \"batchelor\""));
	}

	if(element->GetUserData() != nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Node has user data but it should be empty"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element->FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + std::string(attribute->Name()) + "'"));
	}

	for(const tinyxml2::XMLNode* node = element->FirstChild(); node != nullptr; node = node->NextSibling()) {
		setLineNo(node->GetLineNum());

		const tinyxml2::XMLElement* innerElement = node->ToElement();
		if(innerElement == nullptr) {
			continue;
		}

		setLineNo(innerElement->GetLineNum());
		parseInnerElement(*innerElement);
	}
}

Main& Config::getMain() const noexcept {
	return main;
}

Config::Setting::Setting(Main& main, const std::string& filename, const tinyxml2::XMLElement& element)
: config::Setting(filename, element)
{
	setLineNo(element.GetLineNum());

	if(key == "maximum-tasks-running") {
		if(main.getMaximumTasksRunning() != std::string::npos) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of parameter \"maximum-tasks-running\"."));
		}

		int v = 0;

		try {
			v = std::stoi(evaluate(value, language));
		}
		catch(const std::invalid_argument& e) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(value) + "\"."));
		}
		catch(const std::out_of_range& e) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"" + std::string(value) + "\" is out of range."));
		}
		if(v <= 0) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"" + std::string(value) + "\" is negative."));
		}

		main.setMaximumTasksRunning(v);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + std::string(element.Name()) + "\"."));
	}
}

Config::UserDefinedMetrics::UserDefinedMetrics(Main& main, const std::string& filename, const tinyxml2::XMLElement& element)
: config::Setting(filename, element)
{
	setLineNo(element.GetLineNum());
	main.addUserDefinedMetric(key, value);
}


void Config::parseInnerElement(const tinyxml2::XMLElement& element) {
	setLineNo(element.GetLineNum());
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	const std::string elementName(element.Name());

	if(elementName == "plugin") {
	}
	else if(elementName == "connection") {
	}
	else if(elementName == "setting") {
		Setting(main, getFilename(), element);
	}
	else if(elementName == "user-defined-metric") {
		UserDefinedMetrics(main, getFilename(), element);
	}
	else if(elementName == "event") {
		Event(main, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + elementName + "\"."));
	}
}

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
