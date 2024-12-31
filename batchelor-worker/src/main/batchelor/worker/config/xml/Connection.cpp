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
#include <batchelor/common/plugin/ConnectionFactory.h>

#include <batchelor/worker/config/xml/Connection.h>


#include <esl/io/FilePosition.h>
#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>

#include <tinyxml2.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

Connection::Connection(esl::object::Context& context, Procedure::Settings& mainSettings, const std::string& id, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::ParseElements(filename, element)
{
	parse();

	if(type.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing attribute 'type'"));
	}

	try {
		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::ConnectionFactory>(type, settings));
		if(mainSettings.connectionFactoryIds.insert(id).second == false) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of element <connection> with attribute id=\"" + id + "\"."));
		}
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
}

void Connection::parseUserData(void*) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
}

void Connection::parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) {
	std::string attributeName(attribute.Name());

	if(attributeName == "type") {
		if(!type.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute \"type\"."));
		}
		type = attribute.Value();
		if(type.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute 'type' is invalid"));
		}
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + attributeName + "'"));
	}
}

void Connection::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "setting") {
		Setting(settings, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

//Connection::Setting::Setting(common::config::Server& server, const std::string& filename, const common::config::xml::Element& element)
Connection::Setting::Setting(std::vector<std::pair<std::string, std::string>>& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	settings.emplace_back(std::make_pair(key, value));
}

} /* namespace xml */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
