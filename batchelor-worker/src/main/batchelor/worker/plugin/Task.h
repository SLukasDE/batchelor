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

#ifndef BATCHELOR_WORKER_PLUGIN_TASK_H_
#define BATCHELOR_WORKER_PLUGIN_TASK_H_

#include <batchelor/common/types/State.h>

#include <map>
#include <string>

namespace batchelor {
namespace worker {
namespace plugin {

class Task {
public:
	struct Status {
		common::types::State::Type state;
		int returnCode;
		std::string message; // e.g. exception message
	};

	Task(const std::map<std::string, int>& resources);
	virtual ~Task() = default;

	const std::map<std::string, int>& getResources() const noexcept;

	virtual Status getStatus() const = 0;
	virtual void sendSignal(const std::string& signal) = 0;

private:
	const std::map<std::string, int> resources;
};

} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_TASK_H_ */
