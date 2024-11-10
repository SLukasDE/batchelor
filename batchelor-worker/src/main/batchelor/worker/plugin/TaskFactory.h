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

#ifndef BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_
#define BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/Task.h>

#include <esl/object/Object.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
namespace plugin {

class TaskFactory : public esl::object::Object {
public:
	virtual const std::map<std::string, int>& getResourcesRequired() const = 0;

	virtual bool isBusy(const std::map<std::string, int>& resourcesAvailable) = 0;
	virtual std::unique_ptr<Task> createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const service::schemas::RunConfiguration& runConfiguration) = 0;
};

} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_TASKFACTORY_H_ */
