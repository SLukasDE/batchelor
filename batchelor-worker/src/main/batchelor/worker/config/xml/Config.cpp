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

#include <batchelor/common/config/xml/Element.h>
#include <batchelor/common/config/xml/FilePosition.h>
#include <batchelor/common/Timestamp.h>

#include <batchelor/worker/config/xml/Config.h>
#include <batchelor/worker/config/xml/Connection.h>
#include <batchelor/worker/config/xml/Event.h>

#include <esl/io/FilePosition.h>

#include <stdexcept>
#include <iostream>

namespace batchelor {
namespace worker {
namespace config {
namespace xml {

using common::config::xml::Element;

Config::Config(esl::object::Context& aContext, Procedure::Settings& aSettings, const std::string& filename)
: common::config::xml::File(filename),
  context(aContext),
  settings(aSettings)
{
	parse();
}

void Config::parseInnerElement(const common::config::xml::Element& element) {
	if(element.getElementName() == "plugin") {
	}
	else if(element.getElementName() == "connection") {
		++serverCount;
		std::string id = "batchelor-head-server-" + std::to_string(serverCount);

		Connection(context, settings, id, getFilename(), element);
	}
	else if(element.getElementName() == "event") {
		Event(context, settings, getFilename(), element);
	}
	else if(element.getElementName() == "setting") {
		Setting(settings, getFilename(), element);
	}
	else if(element.getElementName() == "metric") {
		Metric(settings, getFilename(), element);
	}
	else {
		throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Unknown element name \"" + element.getElementName() + "\"."));
	}
}

Config::Metric::Metric(Procedure::Settings& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	settings.metrics.emplace_back(std::make_pair(key, value));
}

Config::Setting::Setting(Procedure::Settings& settings, const std::string& filename, const common::config::xml::Element& element)
: common::config::xml::Setting(filename, element)
{
	parse();

	if(key == "namespace") {
		if(settings.namespaceId.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of parameter \"" + key + "\"."));
		}
		settings.namespaceId = evaluate(value, language);
		if(!settings.namespaceId.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(settings.namespaceId) + "\" for parameter \"" + key + "\"."));
		}
	}
	else if(key == "idle-timeout") {
		if(settings.idleTimeout.count() == 0) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of parameter \"" + key + "\"."));
		}

		try {
			settings.idleTimeout = common::Timestamp::toDuration(evaluate(value, language));
		}
		catch(const std::exception& e) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(value) + "\" for parameter \"" + key + "\". " + e.what()));
		}
		catch(...) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(value) + "\" for parameter \"" + key + "\"."));
		}
	}
	else if(key == "available-timeout") {
		if(settings.availableTimeout.count() == 0) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of parameter \"" + key + "\"."));
		}

		try {
			settings.availableTimeout = common::Timestamp::toDuration(evaluate(value, language));
		}
		catch(const std::exception& e) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(value) + "\" for parameter \"" + key + "\". " + e.what()));
		}
		catch(...) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(value) + "\" for parameter \"" + key + "\"."));
		}
	}
	else if(key == "worker-id") {
		if(settings.workerId.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Multiple definition of parameter \"" + key + "\"."));
		}
		settings.workerId = evaluate(value, language);
		if(!settings.workerId.empty()) {
			throw esl::io::FilePosition::add(getFilename(), getLineNo(), std::runtime_error("Invalid value \"" + std::string(settings.namespaceId) + "\" for parameter \"" + key + "\"."));
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
