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

#define MY_CONSUMER

#include <batchelor/common/config/xml/Setting.h>
#include <batchelor/common/types/State.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/kubectl/Task.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/io/Consumer.h>
#include <esl/io/Input.h>
#ifndef MY_CONSUMER
#include <esl/io/input/String.h>
#endif
#include <esl/io/output/Memory.h>
#include <esl/io/Reader.h>
#include <esl/system/DefaultProcess.h>
#include <esl/system/Environment.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

#include <iostream>
namespace batchelor {
namespace worker {
namespace plugin {
namespace kubectl {
namespace {
Logger logger("batchelor::worker::plugin::kubectl::Task");

#ifdef MY_CONSUMER
class MyConsumer : public esl::io::Consumer {
public:
	MyConsumer() = default;

	bool consume(esl::io::Reader& reader) override {
		char buffer[4096];
		std::size_t count = reader.read(buffer, sizeof(buffer));

		if(count == esl::io::Reader::npos) {
			return false;
		}

		str << std::string(buffer, count);

		return true;
	}

	std::stringstream& getStringStream() noexcept {
		return str;
	}

private:
	std::stringstream str;
};
#endif
}

Task::Task(TaskFactory& aTaskFactoryExec, std::condition_variable& aNotifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& aMetrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration)
: plugin::Task(factorySettings.resourcesRequired),
  taskId(runConfiguration.taskId),
  eventType(runConfiguration.eventType),
  taskFactory(aTaskFactoryExec),
  notifyCV(aNotifyCV),
  taskStatusMutex(notifyMutex)
{
	settings.yamlFile = factorySettings.yamlFile;
	settings.kubectlCmd = factorySettings.kubectlCmd;
	settings.kubectlConfig = factorySettings.kubectlConfig;
	settings.image = factorySettings.image;
	settings.metaNamespace = factorySettings.metaNamespace;
	settings.backoffLimit = factorySettings.backoffLimit;
	settings.serviceAccountName = factorySettings.serviceAccountName;
	settings.imagePullSecrets = factorySettings.imagePullSecrets;
	settings.resourcesRequestsCPU = factorySettings.resourcesRequestsCPU;
	settings.resourcesRequestsMemory = factorySettings.resourcesRequestsMemory;
	settings.resourcesLimitsCPU = factorySettings.resourcesLimitsCPU;
	settings.resourcesLimitsMemory = factorySettings.resourcesLimitsMemory;
	settings.volumes = factorySettings.volumes;
	settings.mounts = factorySettings.mounts;

	bool hasArgs = false;

	status.state = common::types::State::Type::running;

	for(const auto& setting : runConfiguration.settings) {
		if(setting.key == "args") {
			if(hasArgs) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.key + "\".");
			}
			if(factorySettings.argsFlag == TaskFactory::Flag::fixed) {
				throw esl::system::Stacktrace::add(std::runtime_error("It is not allowed to override or extend parameter \"" + setting.key + "\""));
			}
			hasArgs = true;
			settings.args = setting.value;
		}
		else if(setting.key == "env") {
			if(factorySettings.envFlag == TaskFactory::Flag::fixed) {
				throw esl::system::Stacktrace::add(std::runtime_error("It is not allowed to override parameter \"" + setting.key + "\""));
			}
			std::size_t pos = setting.value.find('=');
			std::string key = setting.value.substr(0, pos);
			std::string value;
			if(pos != std::string::npos) {
		        value = setting.value.substr(pos+1);
			}
			if(settings.envs.insert(std::make_pair(key, value)).second == false) {
				throw std::runtime_error("Multiple definition of key \"" + key + "\" for parameter \"env\".");
			}
		}
		else if(setting.key == "cd" || setting.value == "working-directory") {
			if(!settings.cd.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.key + "\""));
			}
			if(factorySettings.cdFlag == TaskFactory::Flag::fixed) {
				throw esl::system::Stacktrace::add(std::runtime_error("It is not allowed to override parameter \"" + setting.key + "\""));
			}
			settings.cd = setting.value;
			if(settings.cd.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.value + "\" for parameter \"" + setting.key + "\"");
			}
		}
		else if(setting.key == "cmd" || setting.key == "executable") {
			if(!settings.cmd.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.key + "\""));
			}
			if(factorySettings.cmdFlag == TaskFactory::Flag::fixed) {
				throw esl::system::Stacktrace::add(std::runtime_error("It is not allowed to override parameter \"" + setting.key + "\""));
			}
			settings.cmd = setting.value;
			if(settings.cmd.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.value + "\" for parameter \"" + setting.key + "\"");
			}
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown parameter key=\"" + setting.key + "\" with value=\"" + setting.value + "\""));
		}
	}

	if(settings.args.empty()) {
		settings.args = factorySettings.args;
	}
	else if(factorySettings.argsFlag == TaskFactory::Flag::extend && !factorySettings.args.empty()) {
		settings.args = factorySettings.args + " " + settings.args;
	}

	if(settings.envs.empty()) {
		settings.envs = factorySettings.envs;
	}
	else {
		for(const auto& env : factorySettings.envs) {
			if(factorySettings.envFlag == TaskFactory::Flag::override) {
				settings.envs.insert(env);
			}
			else if(factorySettings.envFlag == TaskFactory::Flag::extend) {
				settings.envs[env.first] = env.second;
			}
		}
	}

	if(settings.cd.empty()) {
		settings.cd = factorySettings.cd;
	}

	if(settings.cmd.empty()) {
		settings.cmd = factorySettings.cmd;
	}

	/* now we replace the parameters in the envs and args */
	const std::map<std::string, std::string> metrics(aMetrics.begin(), aMetrics.end());

	settings.args = common::config::xml::Setting::generalEvaluate(settings.args, true, metrics);
	for(auto& env : settings.envs) {
		env.second = common::config::xml::Setting::generalEvaluate(env.second, true, metrics);
	}
	settings.cd = common::config::xml::Setting::generalEvaluate(settings.cd, true, metrics);
	settings.cmd = common::config::xml::Setting::generalEvaluate(settings.cmd, true, metrics);
	settings.image = common::config::xml::Setting::generalEvaluate(settings.image, true, metrics);


	/* check if all necessary properties are set */
	if(settings.cd.empty()) {
		throw std::runtime_error("No working directory defined to create a process.");
	}

	if(settings.cmd.empty()) {
		throw std::runtime_error("No executable defined to create a process.");
	}

	if(settings.image.empty()) {
		throw std::runtime_error("No container image defined.");
	}

	thread = std::thread(&Task::run, this);
}

Task::~Task() {
	thread.join();
}

Task::Status Task::getStatus() const {
	return status;
}

void Task::sendSignal(const std::string& signal) {
	if(signal == "CANCEL") {
		sendCancel();
	}
}

void Task::run() {
	try {
		logger.debug << "execute ...\n";
		Status podStatus = runBatch();
		while(true) {
			{
				std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);
				status = podStatus;
			}
			if(podStatus.state != common::types::State::Type::running) {
				break;
			}

			std::this_thread::sleep_for(std::chrono::seconds(settings.checkPodStatusIntervalSec));
			podStatus = getPodStatus();
		}

		logger.debug << "execution done\n";
	}
	catch(const std::exception& e) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		status.state = common::types::State::Type::zombie;
		status.message = e.what();

		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		status.state = common::types::State::Type::zombie;
		status.message = "Execution failed because of unknown exception.";

		logger.warn << "Execution failed because of unknown exception.\n";
	}

	notifyCV.notify_all();
}

std::string Task::getCmd() const noexcept {
	std::string kubectlCmd = settings.kubectlCmd;
	if(!settings.kubectlConfig.empty()) {
		kubectlCmd += " --kubeconfig " + settings.kubectlConfig;
	}
	if(!settings.metaNamespace.empty()) {
		kubectlCmd += " -n " + settings.metaNamespace;
	}
	return kubectlCmd;
}

Task::Status Task::runBatch() const noexcept {
	Status podStatus;

	try {
		std::string deploymentYAML = getDeploymentYAML();
		if(!settings.yamlFile.empty()) {
			std::ofstream(settings.yamlFile) << deploymentYAML;
		}

		esl::io::output::Memory deploymentYamlProducer(deploymentYAML.data(), deploymentYAML.size());

		std::unique_ptr<esl::system::Process> process = esl::system::DefaultProcess::createNative();

#ifdef MY_CONSUMER
		MyConsumer kubectlOutput;
#else
		esl::io::input::String kubectlOutput;
#endif
		(*process)[esl::system::FileDescriptor::getIn()] << esl::io::Output(deploymentYamlProducer);
		(*process)[esl::system::FileDescriptor::getErr()] >> esl::io::Input(kubectlOutput);

		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " apply -f -"));

		if(returnCode == 0) {
			podStatus.state = common::types::State::Type::running;
			podStatus.returnCode = 0;
		}
		else {
			podStatus.state = common::types::State::Type::signaled;
			podStatus.returnCode = 1;
#ifdef MY_CONSUMER
			podStatus.message =  "Applying batch config failed: " + kubectlOutput.getStringStream().str();
#else
			podStatus.message =  "Applying batch config failed: " + kubectlOutput.getString();
#endif
		}

		logger.debug << "execution done, rc=" << returnCode << "\n";
	}
	catch(const std::exception& e) {
		podStatus.state = common::types::State::Type::signaled;
		podStatus.returnCode = 1;
		podStatus.message = "Execution failed because of exception: \"" + std::string(e.what()) + "\"";

		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		podStatus.state = common::types::State::Type::signaled;
		podStatus.returnCode = 1;
		podStatus.message = "Execution failed because of exception.";

		logger.warn << "Execution failed because of unknown exception.\n";
	}

	return podStatus;
}

void Task::sendCancel() const noexcept {
	try {
		std::unique_ptr<esl::system::Process> process = esl::system::DefaultProcess::createNative();
		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " delete job " + taskId));
		taskCanceled = true;
		logger.debug << "canceled task id " << taskId << " and received return code " << returnCode << "\n";
	}
	catch(const std::exception& e) {
		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		logger.warn << "Execution failed because of unknown exception.\n";
	}
}

std::string Task::getDeploymentYAML() const noexcept {
	std::stringstream deploymentYaml;
	deploymentYaml
	<< "apiVersion: batch/v1\n"
	<< "kind: Job\n"
	<< "metadata:\n"
	<< "  name: " << taskId << "\n"
	<< "spec:\n";
	if(!settings.serviceAccountName.empty()) {
		deploymentYaml
		<< "  serviceAccountName: " + settings.serviceAccountName + "\n";
	}
	deploymentYaml
	<< "  backoffLimit: " << std::to_string(settings.backoffLimit) << "\n"
	<< "  completions: 1\n"
	<< "  ttlSecondsAfterFinished: 100\n"
	<< "  template:\n"
	<< "    spec:\n";
	if(!settings.imagePullSecrets.empty()) {
		deploymentYaml << "      imagePullSecrets:\n";
		for(const auto& image : settings.imagePullSecrets) {
			deploymentYaml << "        - name: \"" << image << "\"\n";
		}
	}
	if(!settings.volumes.empty()) {
		deploymentYaml << "      volumes:\n";
		for(const auto& volume : settings.volumes) {
			std::map<std::pair<std::string, std::string>, std::vector<std::pair<std::string, std::string>>> keyPathByNameKinds; //name+kind -> vector<key+path>
			for(const auto& entry : volume.second) {
				keyPathByNameKinds[std::make_pair(entry.name, entry.kind)].push_back(std::make_pair(entry.key, entry.path));
			}
			if(keyPathByNameKinds.empty()){
				continue;
			}

			deploymentYaml << "        - name: " << volume.first << "\n";

			for(const auto& nameKinds : keyPathByNameKinds) {
				if(nameKinds.second.empty()){
					continue;
				}
				deploymentYaml
				<< "          " << nameKinds.first.second << ":\n";
				if(nameKinds.first.second == "secret") {
					deploymentYaml << "            secretName: " << nameKinds.first.first << "\n";
				}
				else {
					deploymentYaml << "            name: " << nameKinds.first.first << "\n";
				}
				deploymentYaml << "            items:\n";
				for(const auto& keyPath : nameKinds.second) {
					deploymentYaml
					<< "              - key: " << keyPath.first << "\n"
					<< "                path: " << keyPath.second << "\n";
				}

			}

		}
	}
	deploymentYaml
	<< "      containers:\n"
	<< "        - name: " << taskId << "\n"
	<< "          image: \"" << settings.image << "\"\n"
	<< "          imagePullPolicy: Always\n"
	<< "          command: [\"" << settings.cmd << "\"]\n";

	if(!settings.args.empty()) {
		deploymentYaml << "          args:\n";
		esl::system::Arguments arguments(settings.args);
		std::size_t argc = arguments.getArgc();
		char** argv = arguments.getArgv();
		for(std::size_t i=0; i<argc; ++i) {
			deploymentYaml << "            - \"" << argv[i] << "\"\n";
		}
	}

	if(!settings.envs.empty()) {
		deploymentYaml << "          env:\n";
		for(const auto& env : settings.envs) {
			deploymentYaml
			<< "          - name: " << env.first << "\n"
			<< "            value: \"" << env.second << "\"\n";
		}
	}

	deploymentYaml
	<< "          resources:\n"
	<< "            requests:\n"
	<< "              cpu: \"" << settings.resourcesRequestsCPU << "\"\n"
	<< "              memory: \"" << settings.resourcesRequestsMemory << "\"\n"
	<< "            limits:\n"
	<< "              cpu: \"" << settings.resourcesLimitsCPU << "\"\n"
	<< "              memory: \"" << settings.resourcesLimitsMemory << "\"\n";
	if(!settings.mounts.empty()) {
		deploymentYaml << "          volumeMounts:\n";
		for(const auto& mount : settings.mounts) {
			deploymentYaml
			<< "            - mountPath: " << mount.mountPath << "\n"
			<< "              name: " << mount.name << "\n"
			<< "              subPath: " << mount.subPath << "\n"
			<< "              readOnly: " << (mount.readOnly ? "true" : "false") << "\n";
		}

	}
	deploymentYaml << "      restartPolicy: Never\n";

	return deploymentYaml.str();
}

Task::Status Task::getPodStatus() {
	Task::Status podStatus;

	/* ******************************************************
	 * First lets check  kubectl [...] descibe job ${TASK_ID}
	 * ******************************************************/
	podStatus = getJobStatus();


	if(podStatus.state == common::types::State::Type::running) {
		std::string message = podStatus.message;

		podStatus = getEventStatus();

		if(podStatus.message.empty()) {
			podStatus.message = message;
		}
	}
	else if(podStatus.message.empty()) {
		podStatus.message = getEventStatus().message;
	}

	return podStatus;
}

Task::Status Task::getJobStatus() {
	/* ******************************************************
	 * First lets check  kubectl [...] descibe job ${TASK_ID}
	 * ******************************************************/

	std::string line;
	std::size_t pos;
	std::string activeStr;
	std::string succeededStr;
	std::string failedStr;

	try {
		std::unique_ptr<esl::system::Process> process = esl::system::DefaultProcess::createNative();

#ifdef MY_CONSUMER
		MyConsumer kubectlOutput;
#else
		esl::io::input::String kubectlOutput;
#endif

		(*process)[esl::system::FileDescriptor::getOut()] >> esl::io::Input(kubectlOutput);

		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " describe job " + taskId));
		if(returnCode != 0) {
			Status podStatus;

			if(taskCanceled) {
				podStatus.state = common::types::State::Type::signaled;
				podStatus.returnCode = 0;
				podStatus.message = "Cancel executed";
			}
			else {
				podStatus.state = common::types::State::Type::zombie;
				podStatus.returnCode = 0;
				podStatus.message = "Could not get description of job";
			}

			return podStatus;
		}

#ifdef MY_CONSUMER
		std::stringstream& ss(kubectlOutput.getStringStream());
#else
		std::stringstream ss(kubectlOutput.getString());
#endif

		bool statusFound = false;
		while(std::getline(ss, line, '\n')) {
			line = esl::utility::String::ltrim(line);

			// Pods Statuses:    1 Active / 0 Succeeded / 0 Failed
			pos = line.find(':');
			if(pos == std::string::npos || line.substr(0, pos) != "Pods Statuses") {
				continue;
			}

			statusFound = true;

			break;
		}

		if(statusFound == false) {
			Status podStatus;

			podStatus.state = common::types::State::Type::zombie;
			podStatus.returnCode = 1;
			podStatus.message = "Could not get status of pod because there is no such line in description";

			return podStatus;
		}

		// 1 Active / 0 Succeeded / 0 Failed
		std::vector<std::string> values = esl::utility::String::split(esl::utility::String::ltrim(line.substr(pos+1)), ' ', true);

		for(std::size_t index = 1; index < values.size(); ++index) {
			if(esl::utility::String::toLower(values[index]) == "active") {
				if(!activeStr.empty()) {
					Status podStatus;

					podStatus.state = common::types::State::Type::zombie;
					podStatus.message = "Multiple definition of 'Succeeded': " + esl::utility::String::ltrim(line.substr(pos+1));
					podStatus.returnCode = 1;

					return podStatus;
				}
				activeStr = values[index-1];
			}
			if(esl::utility::String::toLower(values[index]) == "succeeded") {
				if(!succeededStr.empty()) {
					Status podStatus;

					podStatus.state = common::types::State::Type::zombie;
					podStatus.message = "Multiple definition of 'Succeeded': " + esl::utility::String::ltrim(line.substr(pos+1));
					podStatus.returnCode = 1;

					return podStatus;
				}
				succeededStr = values[index-1];
			}
			if(esl::utility::String::toLower(values[index]) == "failed") {
				if(!failedStr.empty()) {
					Status podStatus;

					podStatus.state = common::types::State::Type::zombie;
					podStatus.message = "Multiple definition of 'Failed': " + esl::utility::String::ltrim(line.substr(pos+1));
					podStatus.returnCode = 1;

					return podStatus;
				}
				failedStr = values[index-1];
			}
		}

		// ... 1 Succeeded / . Failed
		if(succeededStr.empty() == false && std::stoi(succeededStr) > 0) {
			Status podStatus;

			//podStatus.message = esl::utility::String::ltrim(line.substr(pos+1));
			podStatus.state = common::types::State::Type::done;
			podStatus.returnCode = 0;

			return podStatus;
		}

		// ... 7 Failed
		if(failedStr.empty() == false && std::stoi(failedStr) > 0) {
			Status podStatus;

			// 1 Active / 0 Succeeded / 7 Failed
			if(activeStr.empty() == false && std::stoi(activeStr) > 0) {
				podStatus.message = failedStr + " failures";
				podStatus.state = common::types::State::Type::running;
				podStatus.returnCode = 0;
			}
			// 0 Active / 0 Succeeded / 7 Failed
			else {
				podStatus.state = common::types::State::Type::done;
				podStatus.returnCode = 1;
			}

			return podStatus;
		}
	}
	catch(const std::invalid_argument& e) {
		Status podStatus;

		podStatus.state = common::types::State::Type::zombie;
		podStatus.message = "Cannot conversion value '" + succeededStr + "' or '" + failedStr + "', because argument is invalid: " + line;
		podStatus.returnCode = 1;

		return podStatus;
	}
	catch(const std::out_of_range& e) {
		Status podStatus;

		podStatus.state = common::types::State::Type::zombie;
		podStatus.message = "Cannot conversion value '" + succeededStr + "' or '" + failedStr + "', because argument is out of range: " + line;
		podStatus.returnCode = 1;

		return podStatus;
	}
	catch(...) {
		logger.warn << "Execution failed.\n";

		Status podStatus;

		podStatus.state = common::types::State::Type::zombie;
		podStatus.returnCode = 1;
		podStatus.message = "Could not get description of job because process threw an exception";

		return podStatus;
	}

	Status podStatus;

	//podStatus.message = esl::utility::String::ltrim(line.substr(pos+1));
	podStatus.state = common::types::State::Type::running;
	podStatus.returnCode = 0;

	return podStatus;
}

Task::Status Task::getEventStatus() {
	/* *****************************************************
	 * Next lets check  kubectl [...] get events --field-selector type=Warning | grep 187923ba-08c0-4530-8705-4636ec602a14 for "Warning"
	 * ******************************************************/

	try {
		std::unique_ptr<esl::system::Process> process = esl::system::DefaultProcess::createNative();

#ifdef MY_CONSUMER
		MyConsumer kubectlOutput;
#else
		esl::io::input::String kubectlOutput;
#endif

		(*process)[esl::system::FileDescriptor::getOut()] >> esl::io::Input(kubectlOutput);

		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " get events --field-selector type=Warning"));
		if(returnCode != 0) {
			Status podStatus;

			podStatus.state = common::types::State::Type::zombie;
			podStatus.returnCode = 0;
			podStatus.message = "Could not events";

			return podStatus;
		}

#ifdef MY_CONSUMER
		std::stringstream& ss(kubectlOutput.getStringStream());
#else
		std::stringstream ss(kubectlOutput.getString());
#endif
		std::string line;

		while(std::getline(ss, line, '\n')) {
 			line = esl::utility::String::ltrim(line);
			// 3s  Warning  FailedMount  pod/${TASK_ID}-d6z6q  MountVolume.SetUp failed for volume "secret-mounts" : secret "k8s-batch" not found
			std::size_t pos = line.find(taskId);
			if(pos == std::string::npos) {
				continue;
			}
			sendCancel();

			Status podStatus;

			podStatus.state = common::types::State::Type::zombie;
			podStatus.returnCode = 1;
			podStatus.message = line;

			return podStatus;
		}
	}
	catch(...) {
		logger.warn << "Execution failed.\n";

		Status podStatus;

		podStatus.state = common::types::State::Type::zombie;
		podStatus.returnCode = 1;
		podStatus.message = "Could not get events because process threw an exception";

		return podStatus;
	}

	Status podStatus;

	podStatus.state = common::types::State::Type::running;
	podStatus.returnCode = 0;

	return podStatus;
}

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
