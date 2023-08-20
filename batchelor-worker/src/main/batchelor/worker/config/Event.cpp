#include <batchelor/worker/config/Event.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/io/FilePosition.h>
#include <esl/plugin/Registry.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace config {

Event::Event(Main& aMain, const std::string& filename, const tinyxml2::XMLElement& element)
: common::config::Config(filename),
  main(aMain)
{
	setLineNo(element.GetLineNum());
	if(element.GetUserData() != nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Node has user data but it should be empty"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		setLineNo(attribute->GetLineNum());
		const std::string attributeName(attribute->Name());

		if(attributeName == "id") {
			if(id != "") {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute \"id\"."));
			}
			id = attribute->Value();
			if(id == "") {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'id' is invalid"));
			}
		}
		else if(attributeName == "type") {
			if(type != "") {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute \"type\"."));
			}
			type = attribute->Value();
			if(type == "") {
				throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'type' is invalid"));
			}
		}
		else {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + attributeName + "'"));
		}
	}

	if(id.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'id'"));
	}
	if(type.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'type'"));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		parseInnerElement(*innerElement);
	}

	std::unique_ptr<plugin::TaskFactory> taskFactory;
	try {
		taskFactory = esl::plugin::Registry::get().create<plugin::TaskFactory>(type, settings);
	}
	catch(const esl::plugin::exception::PluginNotFound& e) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), e);
	}
	catch(const std::runtime_error& e) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), e);
	}
	catch(const std::exception& e) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), e);
	}
	catch(...) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Could not create a database connection factory with id '" + id + "' for implementation '" + type + "' because an unknown exception occurred."));
	}

	main.addEventType(id, std::move(taskFactory));
}

Event::Setting::Setting(Event& event, const std::string& filename, const tinyxml2::XMLElement& element)
: config::Setting(filename, element)
{
	setLineNo(element.GetLineNum());
	event.settings.push_back(std::make_pair(key, value));
}

Event::Metric::Metric(const std::string& filename, const tinyxml2::XMLElement& element)
: common::config::Config(filename, element.GetLineNum())
{
	setLineNo(element.GetLineNum());

	if(element.GetUserData() != nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
	}

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
		else {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + std::string(attribute->Name()) + "'"));
		}
	}

	if(key.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'key'"));
	}
}

void Event::parseInnerElement(const tinyxml2::XMLElement& element) {
	setLineNo(element.GetLineNum());
	if(element.Name() == nullptr) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element name is empty"));
	}

	const std::string elementName(element.Name());

	if(elementName == "setting") {
		Setting(*this, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + elementName + "\"."));
	}
}

} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
