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

#include <batchelor/head/config/xml/Config.h>

#include <esl/io/FilePosition.h>

#include <stdexcept>

namespace batchelor {
namespace head {
namespace config {
namespace xml {

Config::Config(Procedure::Settings& aSettings, const std::string& filename)
: common::config::xml::File(filename),
  settings(aSettings)
{
	parse();
}

void Config::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "setting") {
		Setting(settings, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

Config::Setting::Setting(Procedure::Settings& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	if(key == "threads") {
		/*
		if(settings.maximumTasksRunning != std::string::npos) {
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

		if(settings.maximumTasksRunning != std::string::npos) {
			throw std::runtime_error("Multiple specification of setting \"maximum-tasks-running\" is not allowed.");
		}

		settings.maximumTasksRunning = v;

		if(settings.maximumTasksRunning == std::string::npos) {
			throw std::runtime_error("Value of setting \"maximum-tasks-running\" must be equal or greater than 0, but lower than " + std::to_string(std::string::npos) + ".");
		}
		*/
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

} /* namespace xml */
} /* namespace config */
} /* namespace head */
} /* namespace batchelor */
