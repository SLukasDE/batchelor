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

#ifndef BATCHELOR_COMMON_AUTH_REQUESTHANDLER_H_
#define BATCHELOR_COMMON_AUTH_REQUESTHANDLER_H_

#include <batchelor/common/auth/UserData.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace common {
namespace auth {

class RequestHandler : public esl::com::http::server::RequestHandler {
public:
	struct Settings {
		explicit Settings(const std::vector<std::pair<std::string, std::string>>& settings);
		explicit Settings(const std::map<std::string, UserData>& users, const std::map<std::string, std::string>& userByPlainApiKey, const std::map<std::string, std::string>& plainBasicAuthByUser);

		std::map<std::string, UserData> users;
		std::map<std::string, std::string> userByPlainApiKey;
		std::map<std::string, std::string> plainBasicAuthByUser;
		std::string realm;
	};

	RequestHandler(const Settings& settings);

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

private:
	const Settings settings;
};

} /* namespace auth */
} /* namespace common */
} /* namespace batchelor */

#endif /* BATCHELOR_COMMON_AUTH_REQUESTHANDLER_H_ */
