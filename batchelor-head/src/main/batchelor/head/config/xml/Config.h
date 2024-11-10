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

#ifndef BATCHELOR_HEAD_CONFIG_XML_CONFIG_H_
#define BATCHELOR_HEAD_CONFIG_XML_CONFIG_H_

#include <batchelor/common/config/xml/Element.h>
#include <batchelor/common/config/xml/File.h>
#include <batchelor/common/config/xml/Setting.h>

#include <batchelor/head/Procedure.h>

#include <string>

namespace batchelor {
namespace head {
namespace config {
namespace xml {

class Config : public common::config::xml::File {
public:
	Config(Procedure::Settings& settings, const std::string& filename);

protected:
	void parseInnerElement(const common::config::xml::Element& element) override;

private:
	struct Setting : public common::config::xml::Setting {
		Setting(Procedure::Settings& settings, const std::string& filename, const common::config::xml::Element& element);
	};

	Procedure::Settings& settings;
};

} /* namespace xml */
} /* namespace config */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_CONFIG_XML_CONFIG_H_ */
