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

#ifndef BATCHELOR_COMMON_PLUGIN_BASIC_CONNECTIONFACTORY_H_
#define BATCHELOR_COMMON_PLUGIN_BASIC_CONNECTIONFACTORY_H_

#include <batchelor/common/plugin/ConnectionFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace plugin {
namespace basic {

class ConnectionFactory : public plugin::ConnectionFactory {
public:
	static std::unique_ptr<plugin::ConnectionFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	ConnectionFactory(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::com::http::client::ConnectionFactory& get() override;

private:
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory;
};

} /* namespace basic */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_BASIC_CONNECTIONFACTORY_H_ */
