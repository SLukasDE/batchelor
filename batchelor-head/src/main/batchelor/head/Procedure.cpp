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

#include <batchelor/head/Logger.h>
#include <batchelor/head/Procedure.h>
#include <batchelor/head/RequestHandler.h>

#include <esl/com/http/server/MHDSocket.h>
#include <esl/com/http/server/Socket.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace head {
namespace {
Logger logger("batchelor::head::Procedure");

std::unique_ptr<esl::com::http::server::RequestHandler> createRequestHandlerSettings(esl::object::Context& context, const Procedure::Settings& settings) {
	std::unique_ptr<RequestHandler> requestHandler(new RequestHandler(settings));
	requestHandler->initializeContext(context);
	return std::unique_ptr<esl::com::http::server::RequestHandler>(requestHandler.release());
}
}
#if 0
Procedure::Settings::Settings(const std::vector<std::pair<std::string, std::string>>& settings) {
	std::chrono::seconds tmpTimeoutZombie = std::chrono::seconds(0);
	std::chrono::seconds tmpTimeoutCleanup = std::chrono::seconds(0);

    for(const auto& setting : settings) {
    	/*
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
        */
        throw esl::system::Stacktrace::add(std::runtime_error("unknown attribute '" + setting.first + "'."));
    }

	if(tmpTimeoutZombie.count() > 0) {
		timeoutZombie = tmpTimeoutZombie;
	}

	if(tmpTimeoutCleanup.count() > 0) {
		timeoutCleanup = tmpTimeoutCleanup;
	}
}
#endif

Procedure::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: requestHandler(createRequestHandlerSettings(context, settings))
{
	for(const auto& id : settings.observerIds) {
		if(observers.emplace(id, std::ref(context.getObject<plugin::Observer>(id))).second == false) {
			throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of observer id \"" + id + "\"."));
		}
	}

	for(const auto& id : settings.socketIds) {
		if(sockets.emplace(id, std::ref(context.getObject<common::plugin::Socket>(id))).second == false) {
			throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of socket id \"" + id + "\"."));
		}
	}
}

Procedure::Procedure(const Settings& aSettings)
: settings(aSettings)
{
	if(settings.socketIds.empty()) {
		throw std::runtime_error("No sockets defined");
	}
}

Procedure::~Procedure() {
	procedureCancel();
}

void Procedure::procedureCancel() {
	mutex.unlock();
}

void Procedure::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

void Procedure::internalProcedureRun(esl::object::Context& context) {
	if(!initializedSettings) {
		logger.warn << "InizializeContext has not been called.\n";
		return;
	}

	mutex.lock();

	logger.info << "Listen...\n";
	for(auto& socket : initializedSettings->sockets) {
		++listeners;
		socket.second.get().get().listen(*initializedSettings->requestHandler, [&]{
				logger.info << "Socket released.\n";
				if((--listeners) == 0) {
					procedureCancel();
				}
		});
	}

	if(listeners.load() > 0) {
		mutex.lock();
	}
	logger.info << "Listen done\n";

	logger.info << "Release...\n";
	for(const auto& socket : initializedSettings->sockets) {
		socket.second.get().get().release();
	}
	logger.info << "Release done\n";

	mutex.unlock();
}

} /* namespace head */
} /* namespace batchelor */
