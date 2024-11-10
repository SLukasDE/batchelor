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

#ifndef BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_
#define BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/com/http/client/ConnectionFactory.h>

#include <memory>
#include <string>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

class TokenFactoryClientCredentials : public TokenFactory {
public:
	TokenFactoryClientCredentials(std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory, const std::string& clientId, const std::string& clientSecret);

	std::string getToken() override;

private:
	std::unique_ptr<esl::com::http::client::ConnectionFactory> connectionFactory;
	const std::string clientId;
	const std::string clientSecret;
};

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_PLUGIN_OIDC_TOKENFACTORYCLIENTCREDENTIALS_H_ */
