#if 0
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

#include <batchelor/common/Timestamp.h>

#include <batchelor/head/Logger.h>
#include <batchelor/head/requesthandler/Engine.h>
#include <batchelor/head/Service.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/database/SQLiteConnectionFactory.h>
#include <esl/object/Value.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace batchelor {
namespace head {
namespace requesthandler {

namespace {
Logger logger("batchelor::head::requesthandler::Engine");
} /* namespace */

Engine::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
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
		else if(setting.first == "zombie-timeout") {
			if(timeoutZombie.count() > 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
			}

			try {
				timeoutZombie = common::Timestamp::toDuration(setting.second);
			}
			catch(const std::exception& e) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'." + e.what()));
			}
		}
		else if(setting.first == "cleanup-timeout") {
			if(timeoutCleanup.count() > 0) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of attribute '" + setting.first + "'."));
			}

			try {
				timeoutCleanup = common::Timestamp::toDuration(setting.second);
			}
			catch(const std::exception& e) {
	            throw esl::system::Stacktrace::add(std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'." + e.what()));
			}
		}
        else {
            throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
        }
    }

	if(timeoutZombie.count() > 0) {
		timeoutZombie = std::chrono::minutes(5);
	}

	if(timeoutCleanup.count() > 0) {
		timeoutCleanup = std::chrono::hours(1);
	}
}

Engine::Settings::Settings(const Procedure::Settings& settings)
: timeoutZombie(settings.timeoutZombie.count() > 0 ? settings.timeoutZombie :std::chrono::minutes(5)),
  timeoutCleanup(settings.timeoutCleanup.count() > 0 ? settings.timeoutCleanup : std::chrono::hours(1)),
  dbConnectionFactoryId(settings.databaseId)
{ }

Engine::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: dbConnectionFactoryPtr(settings.dbConnectionFactoryId.empty() ? esl::database::SQLiteConnectionFactory::createNative(esl::database::SQLiteConnectionFactory::Settings({
		//{{"URI"}, {":memory:"}}
		{{"URI"}, {"file:test?mode=memory"}}
		//{{"URI"}, {"file::memory:?mode=rw"}}
	})) : nullptr),
  dbConnectionFactory(settings.dbConnectionFactoryId.empty() ? *dbConnectionFactoryPtr.get() : context.getObject<esl::database::ConnectionFactory>(settings.dbConnectionFactoryId))
{
	for(const auto& pluginId : settings.pluginIds) {
		plugins.emplace_back(std::ref(context.getObject<plugin::Observer>(pluginId)));
	}
}



Engine::Engine(const Settings& aSettings)
: service::server::RequestHandler([this](const esl::object::Context& context)
		{
			return std::unique_ptr<service::Service>(new Service(context, *this, notifyMutex));
		}),
  settings(aSettings)
{ }

Engine::~Engine() {
	threadStop();
	thread.join();
}

std::unique_ptr<esl::com::http::server::RequestHandler> Engine::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new Engine(Settings(settings)));
}

void Engine::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
	thread = std::thread(&Engine::threadRun, this);
}

esl::database::ConnectionFactory& Engine::getDbConnectionFactory() const noexcept {
	if(!initializedSettings) {
        throw esl::system::Stacktrace::add(std::runtime_error("Object not initialized."));
	}
	return initializedSettings->dbConnectionFactory;
}

void Engine::onUpdateTask(const Dao::Task& task) {
//	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	for(const auto& plugin : initializedSettings->plugins) {
		plugin.get().onUpdateTask(task);
	}
}

void Engine::threadRun() {
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

void Engine::cleanup() {
	auto dbConnection = getDbConnectionFactory().createConnection();
	if(!dbConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("no db connection available."));
	}

	Dao(*dbConnection).cleanup(settings.timeoutZombie, settings.timeoutCleanup);
}

void Engine::threadStop() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		threadStopping = true;
	}
	notifyCV.notify_one();
}

} /* namespace requesthandler */
} /* namespace head */
} /* namespace batchelor */
#endif

