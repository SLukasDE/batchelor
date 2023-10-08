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

#include <batchelor/common/config/xml/FilePosition.h>
#include <batchelor/common/config/xml/Element.h>

#include <batchelor/worker/config/xml/Event.h>
#include <batchelor/worker/config/xml/Config.h>
#include <batchelor/worker/config/xml/Connection.h>

#include <esl/io/FilePosition.h>

#include <stdexcept>
#include <iostream>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

using common::config::xml::Element;

Config::Config(Main::Settings& aSettings, const std::string& filename)
: common::config::xml::File(filename),
  settings(aSettings)
{
	parse();
}

void Config::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "plugin") {
	}
	else if(element.getElementName() == "connection") {
		Connection(settings, getFilename(), element);
	}
	else if(element.getElementName() == "setting") {
		Setting(settings, getFilename(), element);
	}
	else if(element.getElementName() == "metric") {
		Metric(settings, getFilename(), element);
	}
	else if(element.getElementName() == "event") {
		Event(settings, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

Config::Metric::Metric(Main::Settings& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	settings.metrics.emplace_back(std::make_pair(key, value));
}

Config::Setting::Setting(Main::Settings& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	if(key == "maximum-tasks-running") {
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
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple specification of setting \"maximum-tasks-running\" is not allowed."));
		}

		settings.maximumTasksRunning = v;

		if(settings.maximumTasksRunning == std::string::npos) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Value of setting \"maximum-tasks-running\" must be equal or greater than 0, but lower than " + std::to_string(std::string::npos) + "."));
		}
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

} /* namespace xml */
} /* namespace config */
} /* namespace worker */
} /* namespace batchelor */
