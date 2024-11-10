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

#ifndef BATCHELOR_WORKER_PLUGIN_KUBECTL_TASK_H_
#define BATCHELOR_WORKER_PLUGIN_KUBECTL_TASK_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/kubectl/TaskFactory.h>
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
namespace kubectl {

class Task : public plugin::Task {
public:
	struct Settings {
		unsigned int checkPodStatusIntervalSec = 2;

		std::string cd;
		std::string cmd;
		std::string args;
		std::map<std::string, std::string> envs;

		std::string yamlFile;
		std::string kubectlCmd;
		std::string kubectlConfig;
		std::string image;
		std::string metaNamespace;
		int backoffLimit = 0;
		std::string serviceAccountName;
		std::set<std::string> imagePullSecrets;
		std::string resourcesRequestsCPU;
		std::string resourcesRequestsMemory;
		std::string resourcesLimitsCPU;
		std::string resourcesLimitsMemory;
		std::map<std::string, std::vector<TaskFactory::Settings::Volume>> volumes;
		std::vector<TaskFactory::Settings::Mount> mounts;

	};

	Task(TaskFactory& taskFactoryExec, std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration);
	~Task();

	Status getStatus() const override;

	void sendSignal(const std::string& signal) override;

private:
	std::string taskId;
	std::string eventType;
	mutable bool taskCanceled = false;

	TaskFactory& taskFactory;

	std::condition_variable& notifyCV;
	std::mutex& taskStatusMutex;
	Status status;

	Settings settings;

	std::thread thread;

	void run();

	std::string getCmd() const noexcept;
	Status runBatch() const noexcept;
	void sendCancel() const noexcept;
	std::string getDeploymentYAML() const noexcept;
	Status getPodStatus();
	Status getJobStatus();
	Status getEventStatus();
};

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_KUBECTL_TASK_H_ */
