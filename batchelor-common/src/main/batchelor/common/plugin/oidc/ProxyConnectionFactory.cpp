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

#include <batchelor/common/plugin/oidc/ProxyConnection.h>
#include <batchelor/common/plugin/oidc/ProxyConnectionFactory.h>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

ProxyConnectionFactory::ProxyConnectionFactory(esl::com::http::client::ConnectionFactory& aConnectionFactory, TokenFactory& aTokenFactory)
: connectionFactory(aConnectionFactory),
  tokenFactory(aTokenFactory)
{ }

std::unique_ptr<esl::com::http::client::Connection> ProxyConnectionFactory::createConnection() const {
	return std::unique_ptr<esl::com::http::client::Connection>(new ProxyConnection(connectionFactory.createConnection(), tokenFactory));
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
