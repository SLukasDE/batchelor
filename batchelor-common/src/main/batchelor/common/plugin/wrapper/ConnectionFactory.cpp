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

#include <batchelor/common/plugin/wrapper/ConnectionFactory.h>

#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace wrapper {

ConnectionFactory::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "connection-factory-id") {
			if(!connectionFactoryId.empty()) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}

			connectionFactoryId = setting.second;

			if(connectionFactoryId.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(connectionFactoryId.empty()) {
		throw std::runtime_error("Definition of parameter \"connection-factory-id\" is missing");
	}
}

ConnectionFactory::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: connectionFactory(context.getObject<esl::com::http::client::ConnectionFactory>(settings.connectionFactoryId))
{ }

std::unique_ptr<plugin::ConnectionFactory> ConnectionFactory::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<plugin::ConnectionFactory>(new ConnectionFactory(Settings(settings)));
}

ConnectionFactory::ConnectionFactory(const Settings& aSettings)
: settings(aSettings)
{ }

esl::com::http::client::ConnectionFactory& ConnectionFactory::get() {
	if(!initializedSettings) {
		throw std::runtime_error("ConnectionFactory wrapper is not initialized");
	}
	return initializedSettings->connectionFactory;
}

void ConnectionFactory::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

} /* namespace wrapper */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
