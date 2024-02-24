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

#include <batchelor/common/plugin/oidc/TokenFactory.h>

#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <stdexcept>

#include "sergut/JsonDeserializer.h"

namespace batchelor {
namespace common {
namespace plugin {
namespace oidc {

namespace {

struct AccessTokenResponse {
	std::string accessToken;
	std::string tokenType;
	unsigned int expiresIn = 0;
	unsigned int refreshExpiresIn = 0;
	std::string refreshToken;
};

SERGUT_FUNCTION(AccessTokenResponse, data, ar) {
	ar & SERGUT_RENAMED_MMEMBER(data.accessToken, "access_token")
		& SERGUT_RENAMED_MMEMBER(data.tokenType, "token_type")
		& SERGUT_RENAMED_MMEMBER(data.expiresIn, "expires_in")
		& SERGUT_RENAMED_OMEMBER(data.refreshExpiresIn, "refresh_expires_in")
		& SERGUT_RENAMED_OMEMBER(data.refreshToken, "refresh_token");
}

} /* anonymous namespace */

void TokenFactory::processResponse(const esl::com::http::client::Response& response, const std::string& responseBody) {
    if(response.getStatusCode() < 200 || response.getStatusCode() > 299) {
        throw esl::system::Stacktrace::add(std::runtime_error("Received wrong status code  " + std::to_string(response.getStatusCode()) + " from OAuth service while trying to authenticate"));
    }

    AccessTokenResponse accessTokenResponse;
    if(response.getContentType() == esl::utility::MIME::Type::applicationJson) {
        try {
            sergut::JsonDeserializer deSerializer(responseBody);
            accessTokenResponse = deSerializer.deserializeData<AccessTokenResponse>();
        } catch(const std::exception& e) {
            throw esl::system::Stacktrace::add(std::runtime_error(e.what()));
        }
    } else {
        throw esl::system::Stacktrace::add(std::runtime_error("Received not supported response content type \"" + response.getContentType().toString() + "\""));
    }

    tokenType = accessTokenResponse.tokenType;
    accessToken = accessTokenResponse.accessToken;
    accessTokenExpiresAt = std::chrono::steady_clock::now() + std::chrono::seconds(accessTokenResponse.expiresIn);
    refreshToken = accessTokenResponse.refreshToken;
    refreshTokenExpiresAt = std::chrono::steady_clock::now() + std::chrono::seconds(accessTokenResponse.refreshToken.empty() ? 0 : accessTokenResponse.refreshExpiresIn);
}

} /* namespace oidc */
} /* namespace plugin */
} /* namespace common */
} /* namespace batchelor */
