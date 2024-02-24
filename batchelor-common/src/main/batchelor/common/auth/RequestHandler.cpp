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

#include <batchelor/common/auth/RequestHandler.h>
#include <batchelor/common/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/input/Closed.h>
#include <esl/io/Output.h>
#include <esl/io/output/String.h>
#include <esl/object/Value.h>
#include <esl/utility/String.h>
#include <esl/utility/MIME.h>

#include <string>

namespace batchelor {
namespace common {
namespace auth {

namespace {
Logger logger("batchelor::common::auth::RequestHandler");

void addAuthData(esl::com::http::server::RequestContext& requestContext, const std::map<std::string, std::set<UserData::Role>>& rolesByNamespace) {
	esl::object::Value<std::vector<std::pair<std::string, std::string>>>* authDataPtr;
	authDataPtr = requestContext.getObjectContext().findObject<esl::object::Value<std::vector<std::pair<std::string, std::string>>>>("auth-data");

	if(authDataPtr == nullptr) {
		std::unique_ptr<esl::object::Value<std::vector<std::pair<std::string, std::string>>>> authData;
		authData.reset(new esl::object::Value<std::vector<std::pair<std::string, std::string>>>(std::vector<std::pair<std::string, std::string>>()));
		authDataPtr = authData.get();
		requestContext.getObjectContext().addObject("auth-data", std::move(authData));
	}

	for(const auto& namespaceRoles : rolesByNamespace) {
		for(const auto& role : namespaceRoles.second) {
			authDataPtr->get().emplace_back("batchelor", namespaceRoles.first + ":" + UserData::fromRole(role));
		}
	}
}
} /* namespace */

RequestHandler::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "api-key") {
			// basic = <user name>:plain:<api key>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}

			if(values[1] == "plain") {
				userByPlainApiKey[values[2]] = values[0];
			}
			else {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown type \"" + values[1] + "\" in value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
		}
		else if(setting.first == "basic-auth") {
			// basic = <user name>:plain:<pw>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
			auto& password = plainBasicAuthByUser[values[0]];
			if(values[1] == "plain") {
				password = values[2];
			}
			else {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown type \"" + values[1] + "\" in value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
		}
		else if(setting.first == "user") {
			// user = <user name>:<namespace>:<role>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
			UserData& userData = users[values[0]];
			std::set<UserData::Role>& roles = userData.rolesByNamespace[values[1]];
			try {
				roles.insert(UserData::toRole(values[2]));
			}
			catch(...) {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown role \"" + values[2] + "\" for in value \"" + setting.second + "\" at key '" + setting.first + "'."));
			}
		}
		else if(setting.first == "realm") {
			if(!realm.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of key '" + setting.first + "'."));
			}
			realm = setting.second;
			if(realm.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
		}
	}

	if(realm.empty()) {
		realm = "batchelor";
	}
}

RequestHandler::Settings::Settings(const std::map<std::string, UserData>& aUsers, const std::map<std::string, std::string>& aUserByPlainApiKey, const std::map<std::string, std::string>& aPlainBasicAuthByUser)
: users(aUsers),
  userByPlainApiKey(aUserByPlainApiKey),
  plainBasicAuthByUser(aPlainBasicAuthByUser)
{ }

RequestHandler::RequestHandler(const Settings& aSettings)
: settings(aSettings)
{ }

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(Settings(settings)));
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	if(settings.users.empty() && settings.userByPlainApiKey.empty() && settings.plainBasicAuthByUser.empty()) {
		return esl::io::Input();
	}

	try {
		const auto& headers = requestContext.getRequest().getHeaders();
		auto headersIter = headers.begin();
#if 1
		for(;headersIter != headers.end(); ++headersIter) {
			if(esl::utility::String::toLower(headersIter->first) == "authorization") {
				break;
			}
		}
#else
		auto headersIter = headers.find("Authorization");
#endif
		if(headersIter == headers.end()) {
			throw esl::com::http::server::exception::StatusCode(401);
		}

		const std::string& authorizationHeader = headersIter->second;
		std::vector<std::string> authorizationHeaderSplit = esl::utility::String::split(authorizationHeader, ' ', true);

		if(authorizationHeaderSplit.size() < 1) {
			logger.warn << "Authorization header has is empty.\n";
			throw esl::com::http::server::exception::StatusCode(400);
		}

		if(authorizationHeaderSplit[0] == "Bearer") {
			if(authorizationHeaderSplit.size() < 2) {
				logger.warn << "Authorization header has no token. Header should look like \"Authorization: Bearer <token>\".\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}

			if(authorizationHeaderSplit.size() > 2) {
				logger.warn << "Authorization header too many values:  \"" << authorizationHeaderSplit[0];
				for(std::size_t i=1; i<authorizationHeaderSplit.size(); ++i) {
					logger.warn << " " << authorizationHeaderSplit[i];
				}
				logger.warn << "\".\n";
				logger.warn << "Authorization header should look like: \"Authorization: Bearer <token>\".\n";
				logger.warn << "Drop values after <token> and continue.\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}

			const std::string& apiKey = authorizationHeaderSplit[1];

			auto apiKeyIter = settings.userByPlainApiKey.find(apiKey);
			if(apiKeyIter == settings.userByPlainApiKey.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}
			const auto& username = apiKeyIter->second;

			auto usersIter = settings.users.find(username);
			if(usersIter == settings.users.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

			auto& userData = usersIter->second;
			addAuthData(requestContext, userData.rolesByNamespace);
		}
		else if(authorizationHeaderSplit[0] == "Basic") {
			if(authorizationHeaderSplit.size() < 2) {
				logger.warn << "Authorization header has no token. Header should look like \"Authorization: Basic <token>\".\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}

			if(authorizationHeaderSplit.size() > 2) {
				logger.warn << "Authorization header too many values:  \"" << authorizationHeaderSplit[0];
				for(std::size_t i=1; i<authorizationHeaderSplit.size(); ++i) {
					logger.warn << " " << authorizationHeaderSplit[i];
				}
				logger.warn << "\".\n";
				logger.warn << "Authorization header should look like: \"Authorization: Basic <token>\".\n";
				logger.warn << "Drop values after <token> and continue.\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}

			const std::string& authorizationToken = authorizationHeaderSplit[1];
			std::vector<std::string> authorizationTokenSplit = esl::utility::String::split( esl::utility::String::fromBase64(authorizationToken), ':', false);

			if(authorizationTokenSplit.size() < 1) {
				logger.warn << "Basic auth web token has no 'username'. Basic auth token should look like \"<username>:<password>\".\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}
			const std::string& username = authorizationTokenSplit[0];

			if(authorizationTokenSplit.size() < 2) {
				logger.warn << "Basic auth web token has no 'password'. Basic auth token should look like \"<username>:<password>\".\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}
			const std::string& password = authorizationTokenSplit[1];

			if(authorizationTokenSplit.size() >= 3) {
				logger.warn << "Basic auth web token has more arguments than required. Ignore additional arguments.\n";
				throw esl::com::http::server::exception::StatusCode(400);
			}

			auto basicAuthIter = settings.plainBasicAuthByUser.find(username);
			if(basicAuthIter == settings.plainBasicAuthByUser.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}
			const auto& basicAuthPassword = basicAuthIter->second;
			if(basicAuthPassword != password) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

			auto usersIter = settings.users.find(username);
			if(usersIter == settings.users.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

			auto& userData = usersIter->second;
			addAuthData(requestContext, userData.rolesByNamespace);
		}
		else {
			logger.warn << "Authorization header has is not 'Basic' neither 'Bearer', but '" << authorizationHeaderSplit[0] << "'.\n";
			throw esl::com::http::server::exception::StatusCode(400);
		}
	}
	catch(const esl::com::http::server::exception::StatusCode& e) {
		if(e.getStatusCode() != 401) {
			throw;
		}

		std::string str;
		str +=
				"<!DOCTYPE html>\n"
				"<html>\n"
				"  <head>\n"
				"    <meta charset=\"utf-8\">\n"
				"    <title>401 Unauthorized</title>\n"
				"  </head>\n"
				"  <body>\n"
				"401 Unauthorized\n"
				"  </body>\n"
				"</html>\n";

		esl::io::Output output = esl::io::output::String::create(str);
		esl::com::http::server::Response response(401, esl::utility::MIME::Type::textHtml);
		response.addHeader("WWW-Authenticate", "Basic realm=\"" + settings.realm + "\"");

		requestContext.getConnection().send(response, std::move(output));

		return esl::io::input::Closed::create();
	}

	return esl::io::Input();
}

} /* namespace auth */
} /* namespace common */
} /* namespace batchelor */
