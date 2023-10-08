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

#include <batchelor/common/config/Server.h>
#include <batchelor/common/config/xml/Setting.h>

#include <batchelor/worker/config/xml/Connection.h>

#include <esl/io/FilePosition.h>
#include <esl/plugin/Registry.h>

#include <tinyxml2/tinyxml2.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

Connection::Connection(Main::Settings& mainSettings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::ParseElements(filename, element)
{
	parse();

	if(server.url.empty()) {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Missing key 'url'"));
	}

	mainSettings.servers.push_back(server);
}

void Connection::parseUserData(void*) {
	throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Element has user data but it should be empty"));
}

void Connection::parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) {
	std::string attributeName(attribute.Name());

	if(attributeName == "plugin") {
		if(!server.plugin.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of attribute '" + attributeName + "'."));
		}
		server.plugin = attribute.Value();
		if(server.plugin.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value \"\" of attribute '" + attributeName + "' is invalid."));
		}
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown attribute '" + attributeName + "'"));
	}
}

void Connection::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "setting") {
		Setting(server, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

Connection::Setting::Setting(common::config::Server& server, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	if(key == "url") {
		server.url = value;
	}
	else if(key == "username") {
		server.username = value;
	}
	else if(key == "password") {
		server.password = value;
	}
	else {
		server.settings.push_back(std::make_pair(key, value));
	}
}

} /* namespace xml */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
