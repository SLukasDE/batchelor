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

#include <batchelor/common/plugin/oidc/TokenFactoryClientCredentials.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/Request.h>
#include <esl/io/Input.h>
#include <esl/io/input/String.h>
#include <esl/io/output/String.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include <chrono>
#include <stdexcept>

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

TokenFactoryClientCredentials::TokenFactoryClientCredentials(std::unique_ptr<esl::com::http::client::ConnectionFactory> aConnectionFactory, const std::string& aClientId, const std::string& aClientSecret)
: connectionFactory(std::move(aConnectionFactory)),
  clientId(aClientId),
  clientSecret(aClientSecret)
{
	if(!connectionFactory) {
		throw std::runtime_error("HTTP connection factory is empty");
	}
}

std::string TokenFactoryClientCredentials::getToken() {
	if(accessTokenExpiresAt <= std::chrono::steady_clock::now() || accessToken.empty()) {
	    std::map<std::string, std::string> requestHeaders;

	    requestHeaders["Accept"] = esl::utility::MIME::toString(esl::utility::MIME::Type::applicationJson);
	    requestHeaders["Authorization"] = "Basic " + esl::utility::String::toBase64(clientId + ":" + clientSecret, esl::utility::String::base64);

	    esl::com::http::client::Request request("", esl::utility::HttpMethod::Type::httpPost, esl::utility::MIME("application/x-www-form-urlencoded"), requestHeaders);

		std::unique_ptr<esl::com::http::client::Connection> connection = connectionFactory->createConnection();
		if(!connection) {
			throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
		}

	    esl::io::input::String consumerString;
	    esl::com::http::client::Response response = connection->send(std::move(request), esl::io::output::String::create("grant_type=client_credentials"), esl::io::Input(consumerString));
	    processResponse(response, consumerString.getString());
	}

	if(accessTokenExpiresAt <= std::chrono::steady_clock::now() || accessToken.empty()) {
		throw esl::system::Stacktrace::add(std::runtime_error("Refresh failed."));
	}

	return accessToken;
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
