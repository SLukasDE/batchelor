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

#ifndef BATCHELOR_HEAD_JERRYPLUGIN_REQUESTHANDLER_H_
#define BATCHELOR_HEAD_JERRYPLUGIN_REQUESTHANDLER_H_

#include <batchelor/head/common/RequestHandler.h>

#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace head {
namespace jerryplugin {

class RequestHandler : public common::RequestHandler, public esl::object::InitializeContext {
public:
	static std::unique_ptr<esl::com::http::server::RequestHandler> create(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& context) override;

private:
	struct Settings {
		std::string dbConnectionFactoryId;
		std::set<std::string> pluginIds;
	};

	struct InitializedSettings : common::RequestHandler::InitializedSettings {
		InitializedSettings(esl::object::Context& context, const Settings& settings);
	};

	RequestHandler(Settings&& settings);

	const Settings settings;
};

} /* namespace jerryplugin */
} /* namespace head */
} /* namespace batchelor */

#endif /* BATCHELOR_HEAD_JERRYPLUGIN_REQUESTHANDLER_H_ */
