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

#include <batchelor/worker/Plugin.h>
#include <batchelor/worker/plugin/exec/TaskFactory.h>
#include <batchelor/worker/plugin/kubectl/TaskFactory.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <memory>

namespace batchelor {
namespace worker {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin<plugin::TaskFactory>(
			"exec",
			plugin::exec::TaskFactory::create);

	registry.addPlugin<plugin::TaskFactory>(
			"kubectl",
			plugin::kubectl::TaskFactory::create);
}

} /* namespace worker */
} /* namespace batchelor */
