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

#include <batchelor/ui/Logger.h>
#include <batchelor/ui/Procedure.h>
#include <batchelor/ui/RequestHandler.h>

#include <esl/com/http/server/MHDSocket.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/Socket.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>

#include <stdexcept>
#include <utility>

namespace batchelor {
namespace ui {
namespace {
Logger logger("batchelor::ui::Procedure");

class MyRequestHandler : public esl::com::http::server::RequestHandler, public esl::object::InitializeContext {
public:
	MyRequestHandler(const Procedure::Settings& settings);

	void initializeContext(esl::object::Context& context) override;

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

private:
	common::auth::RequestHandler requestHandlerAuth;
	ui::RequestHandler requestHandlerEngine;
};

MyRequestHandler::MyRequestHandler(const Procedure::Settings& settings)
: requestHandlerAuth(common::auth::RequestHandler::Settings(settings.users, settings.userByPlainApiKey, settings.plainBasicAuthByUser)),
  requestHandlerEngine(ui::RequestHandler::Settings(settings))
{ }

void MyRequestHandler::initializeContext(esl::object::Context& context) {
	requestHandlerEngine.initializeContext(context);
}

esl::io::Input MyRequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	auto rv = requestHandlerAuth.accept(requestContext);
	if(rv) {
		return rv;
	}
	return requestHandlerEngine.accept(requestContext);
}

std::unique_ptr<esl::com::http::server::RequestHandler> createRequestHandler(esl::object::Context& context, const Procedure::Settings& settings) {
	std::unique_ptr<MyRequestHandler> requestHandler(new MyRequestHandler(settings));
	requestHandler->initializeContext(context);
	return std::unique_ptr<esl::com::http::server::RequestHandler>(requestHandler.release());
}

}

Procedure::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings)
: requestHandler(createRequestHandler(context, settings))
{
	for(const auto& connectionId : settings.connectionFactoryIds) {
		common::plugin::ConnectionFactory& connectionFactory = context.getObject<common::plugin::ConnectionFactory>(connectionId);
		connectionFactories.emplace_back(connectionId, std::ref(connectionFactory));
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

} /* namespace ui */
} /* namespace batchelor */
