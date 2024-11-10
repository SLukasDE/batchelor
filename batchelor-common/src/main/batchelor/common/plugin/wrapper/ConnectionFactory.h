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

#ifndef BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_

#include <batchelor/common/plugin/ConnectionFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Context.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace plugin {
namespace wrapper {

class ConnectionFactory : public plugin::ConnectionFactory, esl::object::InitializeContext {
public:
	static std::unique_ptr<plugin::ConnectionFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	struct Settings {
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);

		std::string connectionFactoryId;
	};

	ConnectionFactory(const Settings& settings);

	esl::com::http::client::ConnectionFactory& get() override;

	void initializeContext(esl::object::Context& context) override;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		esl::com::http::client::ConnectionFactory& connectionFactory;
	};

	const Settings& settings;
	std::unique_ptr<InitializedSettings> initializedSettings;
};

} /* namespace wrapper */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_WRAPPER_CONNECTIONFACTORY_H_ */
