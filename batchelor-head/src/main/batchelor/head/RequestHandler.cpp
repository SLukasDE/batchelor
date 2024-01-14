/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#include <batchelor/head/RequestHandler.h>
#include <batchelor/head/Service.h>
#include <batchelor/head/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/object/Value.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace head {

namespace {
Logger logger("batchelor::head::RequestHandler");
} /* namespace */

RequestHandler::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	std::chrono::seconds tmpTimeoutZombie = std::chrono::seconds(0);
	std::chrono::seconds tmpTimeoutCleanup = std::chrono::seconds(0);

    for(const auto& setting : settings) {
        if(setting.first == "db-connection-factory") {
            if(!dbConnectionFactoryId.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("multiple definition of attribute '" + setting.first + "'."));
            }
            dbConnectionFactoryId = setting.second;
            if(dbConnectionFactoryId.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"\" for attribute '" + setting.first + "'."));
            }
        }
        else if(setting.first == "plugin-id") {
            if(setting.second.empty()) {
                throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"\" for attribute '" + setting.first + "'."));
            }
            if(!pluginIds.insert(setting.second).second) {
                throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of value \"" + setting.second + "\" for attribute '" + setting.first + "'."));
            }
        }
        else {
            throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
        }
    }

	if(tmpTimeoutZombie.count() > 0) {
		timeoutZombie = tmpTimeoutZombie;
	}

	if(tmpTimeoutCleanup.count() > 0) {
		timeoutCleanup = tmpTimeoutCleanup;
	}
}

RequestHandler::Settings::Settings(const Procedure::Settings& settings)
: users(settings.users),
  timeoutZombie(settings.timeoutZombie.count() > 0 ? settings.timeoutZombie :std::chrono::minutes(5)),
  timeoutCleanup(settings.timeoutCleanup.count() > 0 ? settings.timeoutCleanup : std::chrono::hours(1)),
  dbConnectionFactoryId(settings.databaseId)
{
}

RequestHandler::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: dbConnectionFactory(context.getObject<esl::database::ConnectionFactory>(settings.dbConnectionFactoryId))
{
	for(const auto& pluginId : settings.pluginIds) {
		plugins.emplace_back(std::ref(context.getObject<plugin::Observer>(pluginId)));
	}
}

RequestHandler::RequestHandler(const Settings& aSettings)
: service::server::RequestHandler([this](const esl::object::Context& context)
		{
			return std::unique_ptr<service::Service>(new Service(context, *this));
		}),
  settings(aSettings)
{ }

RequestHandler::~RequestHandler() {
	threadStop();
	thread.join();
}

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(Settings(settings)));
}

void RequestHandler::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
	thread = std::thread(&RequestHandler::threadRun, this);
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	addRolesByNamespaceValue(requestContext);
	return service::server::RequestHandler::accept(requestContext);
}

esl::database::ConnectionFactory& RequestHandler::getDbConnectionFactory() const noexcept {
	if(!initializedSettings) {
        throw esl::system::Stacktrace::add(std::runtime_error("Object not initialized."));
	}
	return initializedSettings->dbConnectionFactory;
}

void RequestHandler::onUpdateTask(const Dao::Task& task) {
	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	for(const auto& plugin : initializedSettings->plugins) {
		plugin.get().onUpdateTask(task);
	}
}

void RequestHandler::addRolesByNamespaceValue(esl::com::http::server::RequestContext& requestContext) const {
	if(settings.users.empty()) {
		return;
	}

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

	if(authorizationHeaderSplit[0] != "Basic") {
		logger.warn << "Authorization header has is not 'Basic', but '" << authorizationHeaderSplit[0] << "'.\n";
		throw esl::com::http::server::exception::StatusCode(400);
	}

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

	std::unique_ptr<esl::object::Value<std::map<std::string, std::set<Procedure::Settings::Role>>>> rolesByNamespaceValuePtr;
	rolesByNamespaceValuePtr.reset(new esl::object::Value<std::map<std::string, std::set<Procedure::Settings::Role>>>(userData.rolesByNamespace));
	requestContext.getObjectContext().addObject("rolesByNamespaceValue", std::move(rolesByNamespaceValuePtr));
}

void RequestHandler::threadRun() {
	if(!initializedSettings) {
		logger.error << "Internal error: initializedSetting == nullptr\n";
		return;
	}

	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	bool doWait = false;
	while(!threadStopping) {
		if(doWait) {
			notifyCV.wait_for(lockNotifyMutex, std::chrono::milliseconds(5000));
		}

		doWait = true;

		try {
			logger.debug << "thread\n";

			cleanup();

			for(const auto& plugin : initializedSettings->plugins) {
				plugin.get().timerEvent();
			}
		}
		catch(...) { }
	}
}

void RequestHandler::cleanup() {
	auto dbConnection = getDbConnectionFactory().createConnection();
	if(!dbConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("no db connection available."));
	}

	Dao(*dbConnection).cleanup(settings.timeoutZombie, settings.timeoutCleanup);
}

void RequestHandler::threadStop() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		threadStopping = true;
	}
	notifyCV.notify_one();
}

} /* namespace head */
} /* namespace batchelor */
