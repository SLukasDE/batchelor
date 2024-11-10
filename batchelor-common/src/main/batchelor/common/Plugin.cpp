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

#include <batchelor/common/Plugin.h>
#include <batchelor/common/plugin/basic/ConnectionFactory.h>
#include <batchelor/common/plugin/basic/Socket.h>
#include <batchelor/common/plugin/ConnectionFactory.h>
#include <batchelor/common/plugin/oidc/ConnectionFactory.h>
#include <batchelor/common/plugin/Socket.h>
#include <batchelor/common/plugin/wrapper/ConnectionFactory.h>

#include <esl/object/Object.h>

namespace batchelor {
namespace common {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin("basic", plugin::basic::Socket::create);
	registry.addPlugin<esl::object::Object, plugin::Socket, plugin::basic::Socket::create>("batchelor-socket-basic");

	registry.addPlugin("basic", plugin::basic::ConnectionFactory::create);
	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::basic::ConnectionFactory::create>("batchelor-connection-basic");

	registry.addPlugin("oidc", plugin::oidc::ConnectionFactory::create);
	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::oidc::ConnectionFactory::create>("batchelor-connection-oidc");

	registry.addPlugin<esl::object::Object, plugin::ConnectionFactory, plugin::wrapper::ConnectionFactory::create>("batchelor-connection-wrapper");
}

} /* namespace common */
} /* namespace batchelor */
