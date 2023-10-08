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

#include <batchelor/worker/config/xml/Event.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/io/FilePosition.h>
#include <esl/plugin/Registry.h>

#include <tinyxml2/tinyxml2.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

Event::Event(Main::Settings& mainSettings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::ParseElements(filename, element)
{
	parse();

	if(event.id.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'id'"));
	}
	if(event.type.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'type'"));
	}

	std::unique_ptr<plugin::TaskFactory> taskFactory;
	try {
		taskFactory = esl::plugin::Registry::get().create<plugin::TaskFactory>(event.type, event.settings);
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
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Could not create a database connection factory with id '" + event.id + "' for implementation '" + event.type + "' because an unknown exception occurred."));
	}

	mainSettings.events.push_back(event);
}

void Event::parseUserData(void*) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
}

void Event::parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) {
	//setLineNo(attribute->GetLineNum());
	const std::string attributeName(attribute.Name());

	if(attributeName == "id") {
		if(!event.id.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute \"id\"."));
		}
		event.id = attribute.Value();
		if(event.id.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'id' is invalid"));
		}
	}
	else if(attributeName == "type") {
		if(!event.type.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute \"type\"."));
		}
		event.type = attribute.Value();
		if(event.type.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'type' is invalid"));
		}
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + attributeName + "'"));
	}
}

void Event::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "setting") {
		Setting(event.settings, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

Event::Setting::Setting(std::vector<std::pair<std::string, std::string>>& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	settings.emplace_back(std::make_pair(key, value));
}

} /* namespace xml */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
