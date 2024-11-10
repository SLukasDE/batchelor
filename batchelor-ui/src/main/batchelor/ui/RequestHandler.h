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

#ifndef BATCHELOR_UI_REQUESTHANDLER_H_
#define BATCHELOR_UI_REQUESTHANDLER_H_

#include <batchelor/common/auth/UserData.h>
#include <batchelor/common/plugin/ConnectionFactory.h>

#include <batchelor/service/Service.h>

#include <batchelor/ui/Procedure.h>

#include <esl/com/http/client/Connection.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace ui {

class RequestHandler : public esl::com::http::server::RequestHandler, public esl::object::InitializeContext {
public:
	struct Settings {
		Settings(const std::vector<std::pair<std::string, std::string>>& settings);
		Settings(const Procedure::Settings& settings);

		std::string namespaceId = "default";
		std::set<std::string> connectionFactoryIds;
	};

	RequestHandler(const Settings& settings);

	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& context) override;

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

	std::unique_ptr<esl::com::http::client::Connection> createHTTPConnection() const;

private:
	struct InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);

		std::vector<std::pair<std::string, std::reference_wrapper<common::plugin::ConnectionFactory>>> connectionFactories;
	};

	esl::io::Input responseShowTask(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::set<common::auth::UserData::Role>& roles, const std::string& taskId) const;
	esl::io::Input responseShowTasks(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::set<common::auth::UserData::Role>& roles) const;
	esl::io::Input responseSendEvent(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::set<common::auth::UserData::Role>& roles, const std::string& eventType) const;
	esl::io::Input responseSendSignal(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::set<common::auth::UserData::Role>& roles) const;
	esl::io::Input responseShowEventTypes(esl::com::http::server::RequestContext& requestContext, service::Service& service, const std::set<common::auth::UserData::Role>& roles) const;
	esl::io::Input responseMainPage(esl::com::http::server::RequestContext& requestContext, const std::set<common::auth::UserData::Role>& roles) const;

	const Settings settings;
	std::unique_ptr<InitializedSettings> initializedSettings;

	mutable std::size_t nextConnectionFactory = 0;
	mutable common::plugin::ConnectionFactory* httpConnectionFactory = nullptr;
};

} /* namespace ui */
} /* namespace batchelor */

#endif /* BATCHELOR_UI_REQUESTHANDLER_H_ */
