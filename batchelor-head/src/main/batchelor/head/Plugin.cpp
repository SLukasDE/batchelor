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
#include <batchelor/common/Plugin.h>

#include <batchelor/head/Plugin.h>
#include <batchelor/head/RequestHandler.h>

namespace batchelor {
namespace head {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	common::Plugin::install(esl::plugin::Registry::get(), nullptr);

	registry.addPlugin("batchelor-auth", common::auth::RequestHandler::create);
	registry.addPlugin("batchelor-head", RequestHandler::create);
}

} /* namespace head */
} /* namespace batchelor */
