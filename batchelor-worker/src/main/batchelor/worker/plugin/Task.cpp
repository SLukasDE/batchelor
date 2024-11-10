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

#include <batchelor/worker/plugin/Task.h>

namespace batchelor {
namespace worker {
namespace plugin {

Task::Task(const std::map<std::string, int>& aResources)
: resources(aResources)
{ }

const std::map<std::string, int>& Task::getResources() const noexcept {
	return resources;
}

} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
