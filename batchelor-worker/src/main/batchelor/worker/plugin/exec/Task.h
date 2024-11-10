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

#ifndef BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_
#define BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/exec/TaskFactory.h>
#include <batchelor/worker/plugin/Task.h>

#include <esl/system/Arguments.h>
#include <esl/system/Process.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
namespace plugin {
namespace exec {

class Task : public plugin::Task {
public:
	struct Settings {
		std::string args;
		std::map<std::string, std::string> envs;
		std::string cd;
		std::string cmd;
	};

	Task(TaskFactory& taskFactoryExec, std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration);
	~Task();

	Status getStatus() const override;

	void sendSignal(const std::string& signal) override;

private:
	TaskFactory& taskFactory;

	std::condition_variable& notifyCV;
	std::mutex& taskStatusMutex;
	Status status;

	Settings settings;

	std::unique_ptr<esl::system::Process> process;
	esl::system::Arguments arguments;

	std::thread thread;

	void run();
};

} /* namespace exec */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_EXEC_TASK_H_ */
