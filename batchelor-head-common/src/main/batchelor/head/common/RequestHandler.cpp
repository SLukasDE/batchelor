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

#include <batchelor/head/common/RequestHandler.h>
#include <batchelor/head/common/Service.h>
#include <batchelor/head/common/Logger.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace head {
namespace common {

namespace {
Logger logger("batchelor::head::common::RequestHandler");
} /* namespace */

RequestHandler::InitializedSettings::InitializedSettings(esl::database::ConnectionFactory& aDbConnectionFactory, std::vector<std::reference_wrapper<Plugin>> aPlugins)
: dbConnectionFactory(aDbConnectionFactory),
  plugins(std::move(aPlugins))
{ }

RequestHandler::RequestHandler(esl::database::ConnectionFactory& dbConnectionFactory, std::vector<std::reference_wrapper<Plugin>> plugins)
: RequestHandler()
{
	initializedSettings.reset(new InitializedSettings(dbConnectionFactory, plugins));
}

RequestHandler::RequestHandler()
: service::server::RequestHandler([this](const esl::object::Context& context)
		{
			return std::unique_ptr<service::Service>(new Service(context, *this));
		}),
  thread(&RequestHandler::threadRun, this)
{ }

RequestHandler::~RequestHandler() {
	threadStop();
	thread.join();
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
	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);

	bool doWait = false;
	while(!threadStopping) {
		if(doWait) {
			notifyCV.wait_for(lockNotifyMutex, std::chrono::milliseconds(5000));
		}

		doWait = true;

		try {
			if(!initializedSettings) {
				logger.error << "Internal error: initializedSetting == nullptr\n";
				break;
			}

			std::cout << "thread\n";
			for(const auto& plugin : initializedSettings->plugins) {
				plugin.get().timerEvent();
			}
		}
		catch(...) {
		}
	}

}

void RequestHandler::threadStop() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		threadStopping = true;
	}
	notifyCV.notify_one();
}

} /* namespace common */
} /* namespace head */
} /* namespace batchelor */
