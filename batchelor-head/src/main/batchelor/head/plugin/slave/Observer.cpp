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

#include <batchelor/head/plugin/slave/Observer.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace head {
namespace plugin {
namespace slave {

Observer::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "database-id") {
			if(dbConnectionFactoryId.empty() == false) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}
			dbConnectionFactoryId = setting.second;
			if(dbConnectionFactoryId.empty()) {
				throw std::runtime_error("Value \"" + setting.second + "\" of attribute '" + setting.first + "' is invalid");
			}
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}

	if(dbConnectionFactoryId.empty()) {
		throw std::runtime_error("Definition of parameter \"database-id\" is required.");
	}
}

std::unique_ptr<esl::object::Object> Observer::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Object>(new Observer(Settings(settings)));
}

Observer::Observer(const Settings& aSettings)
: settings(aSettings)
{ }

void Observer::onUpdateTask(const Dao::Task& task) {

}

void Observer::timerEvent() {

}

void Observer::initializeContext(esl::object::Context& context) {
	initializedSettings = std::unique_ptr<InitializedSettings>(new  InitializedSettings(context, settings));

}

Observer::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings) {

};

} /* namespace slave */
} /* namespace plugin */
} /* namespace head */
} /* namespace batchelor */
