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

#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/Output.h>

#include <functional>
#include <memory>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class ProxyConnection : public esl::com::http::client::Connection {
public:
	ProxyConnection(std::unique_ptr<esl::com::http::client::Connection> connection, TokenFactory& tokenFactory);

	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, std::function<esl::io::Input (const esl::com::http::client::Response&)> createInput) const override;
	esl::com::http::client::Response send(const esl::com::http::client::Request& request, esl::io::Output output, esl::io::Input input) const override;

private:
	std::unique_ptr<esl::com::http::client::Connection> connection;
	TokenFactory& tokenFactory;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_PROXYCONNECTION_H_ */
