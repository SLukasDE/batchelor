#include <batchelor/head/Logger.h>
#include <batchelor/head/requesthandler/Auth.h>

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
namespace head {
namespace requesthandler {

namespace {
Logger logger("batchelor::head::requesthandler::Auth");

void addAuthData(esl::com::http::server::RequestContext& requestContext, const std::map<std::string, std::set<Procedure::Settings::Role>>& rolesByNamespace) {
#if 1
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
			switch(role) {
			case Procedure::Settings::Role::execute:
				authDataPtr->get().emplace_back("batchelor", namespaceRoles.first + ":execute");
				break;
			case Procedure::Settings::Role::readOnly:
				authDataPtr->get().emplace_back("batchelor", namespaceRoles.first + ":read-only");
				break;
			case Procedure::Settings::Role::worker:
				authDataPtr->get().emplace_back("batchelor", namespaceRoles.first + ":worker");
				break;
			}
		}
	}
#else
	std::unique_ptr<esl::object::Value<std::map<std::string, std::set<Procedure::Settings::Role>>>> rolesByNamespaceValuePtr;
	rolesByNamespaceValuePtr.reset(new esl::object::Value<std::map<std::string, std::set<Procedure::Settings::Role>>>(rolesByNamespace));
	requestContext.getObjectContext().addObject("rolesByNamespaceValue", std::move(rolesByNamespaceValuePtr));
#endif
}
} /* namespace */

Auth::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "api-key") {
			// api-key = <api key>:<namespace>:<role>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
			Procedure::Settings::APIKeyData& apiKeyData = apiKeys[values[0]];
			apiKeyData.apiKey = values[0];
			std::set<Procedure::Settings::Role>& roles = apiKeyData.rolesByNamespace[values[1]];
			if(values[2] == "execute") {
				roles.insert(Procedure::Settings::Role::execute);
			}
			else if(values[2] == "read-only") {
				roles.insert(Procedure::Settings::Role::readOnly);
			}
			else if(values[2] == "worker") {
				roles.insert(Procedure::Settings::Role::worker);
			}
			else {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown role \"" + values[2] + "\" for in value \"" + setting.second + "\" at key '" + setting.first + "'."));
			}
		}
		else if(setting.first == "user") {
			// user = <user name>:<namespace>:<role>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
			Procedure::Settings::UserData& userData = users[values[0]];
			userData.userName = values[0];
			std::set<Procedure::Settings::Role>& roles = userData.rolesByNamespace[values[1]];
			if(values[2] == "execute") {
				roles.insert(Procedure::Settings::Role::execute);
			}
			else if(values[2] == "read-only") {
				roles.insert(Procedure::Settings::Role::readOnly);
			}
			else if(values[2] == "worker") {
				roles.insert(Procedure::Settings::Role::worker);
			}
			else {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown role \"" + values[2] + "\" for in value \"" + setting.second + "\" at key '" + setting.first + "'."));
			}
		}
		else if(setting.first == "basic-auth") {
			// basic = <user name>:plain:<pw>
			auto values = esl::utility::String::split(setting.second, ':', false);
			if(values.size() != 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" of key '" + setting.first + "'."));
			}
			Procedure::Settings::UserData& userData = users[values[0]];
			userData.userName = values[0];
			if(values[1] == "plain") {
				userData.pw = values[2];
			}
			else {
				throw esl::system::Stacktrace::add(std::runtime_error("Unknown pw type \"" + values[1] + "\" for in value \"" + setting.second + "\" at key '" + setting.first + "'."));
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

Auth::Settings::Settings(const Procedure::Settings& settings)
: apiKeys(settings.apiKeys),
  users(settings.users)
{ }

Auth::Auth(const Settings& aSettings)
: settings(aSettings)
{ }

std::unique_ptr<esl::com::http::server::RequestHandler> Auth::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new Auth(Settings(settings)));
}

esl::io::Input Auth::accept(esl::com::http::server::RequestContext& requestContext) const {
	if(settings.apiKeys.empty() && settings.users.empty()) {
		return esl::io::Input();
	}

	try {
		const auto& headers = requestContext.getRequest().getHeaders();
		auto headersIter = headers.find("Authorization");
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
			auto apiKeyIter = settings.apiKeys.find(apiKey);
			if(apiKeyIter == settings.apiKeys.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

			auto& apiKeyData = apiKeyIter->second;
			addAuthData(requestContext, apiKeyData.rolesByNamespace);
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

			auto usersIter = settings.users.find(username);
			if(usersIter == settings.users.end()) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

			auto& userData = usersIter->second;
			if(userData.pw != password) {
				throw esl::com::http::server::exception::StatusCode(401);
			}

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

} /* namespace requesthandler */
} /* namespace head */
} /* namespace batchelor */
