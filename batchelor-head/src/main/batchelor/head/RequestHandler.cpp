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

#include <esl/system/Stacktrace.h>

#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace head {

namespace {
Logger logger("batchelor::head::RequestHandler");
} /* namespace */

RequestHandler::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
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
			for(const auto& plugin : initializedSettings->plugins) {
				plugin.get().timerEvent();
			}
		}
		catch(...) { }
	}
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
