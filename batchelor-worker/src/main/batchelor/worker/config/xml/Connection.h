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

#ifndef BATCHELOR_WORKER_CONFIG_XML_CONNECTION_H_
#define BATCHELOR_WORKER_CONFIG_XML_CONNECTION_H_

#include <batchelor/common/config/Server.h>
#include <batchelor/common/config/xml/Element.h>
#include <batchelor/common/config/xml/ParseElements.h>

#include <batchelor/worker/Main.h>

#include <tinyxml2/tinyxml2.h>

#include <string>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

class Connection : public common::config::xml::ParseElements {
public:
	Connection(Main::Settings& mainSettings, const std::string& filename, const common::config::xml::Element& element);

protected:
	void parseUserData(void*) override;
	void parseInnerAttribute(const tinyxml2::XMLAttribute& attribute) override;
	void parseInnerElement(const common::config::xml::Element& element) override;

private:
	struct Setting : public common::config::xml::Setting {
		Setting(common::config::Server& server, const std::string& filename, const common::config::xml::Element& element);
	};

	common::config::Server server;
};

} /* namespace args */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_CONFIG_XML_CONNECTION_H_ */
