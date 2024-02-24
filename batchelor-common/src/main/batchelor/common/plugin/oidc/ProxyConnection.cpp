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

#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

ProxyConnection::ProxyConnection(std::unique_ptr<esl::com::http::client::Connection> aConnection, TokenFactory& aTokenFactory)
: connection(std::move(aConnection)),
  tokenFactory(aTokenFactory)
{
	if(!connection) {
		throw std::runtime_error("HTTP connection is empty");
	}
}

esl::com::http::client::Response ProxyConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const {
	esl::com::http::client::Request request(aRequest);
	request.addHeader("Authorization", "Bearer " + tokenFactory.getToken());

	return connection->send(request, std::move(output), createInput);
}

esl::com::http::client::Response ProxyConnection::send(const esl::com::http::client::Request& aRequest, esl::io::Output output, esl::io::Input input) const {
	esl::com::http::client::Request request(aRequest);
	request.addHeader("Authorization", "Bearer " + tokenFactory.getToken());

	return connection->send(request, std::move(output), std::move(input));
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
