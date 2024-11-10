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

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/kubectl/Task.h>
#include <batchelor/worker/plugin/kubectl/TaskFactory.h>

#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace plugin {
namespace kubectl {
namespace {
Logger logger("batchelor::worker::plugin::kubectl::TaskFactory");
}


TaskFactory::TaskFactory(Settings aSettings)
: settings(std::move(aSettings))
{
	if(settings.cmd == "") {
		throw esl::system::Stacktrace::add(std::runtime_error("Parameter \"executable\" is required"));
	}
	if(settings.cd == "") {
		logger.warn << "No working directory is specified.\n";
	}
}

std::unique_ptr<plugin::TaskFactory> TaskFactory::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;

	//bool hasMetricsPolicy = false;
	bool hasArgs = false;
	bool hasArgsFlag = false;
	bool hasEnvFlagGlobal = false;
	bool hasEnvFlag = false;
	bool hasCdFlag = false;
	bool hasCmdFlag = false;
	bool hasBackoffLimit = false;

	Settings::Volume* volumePtr = nullptr;
	Settings::Mount* mountPtr = nullptr;

	for(const auto& setting : aSettings) {
		if(setting.first.size() > 18 && setting.first.substr(0, 18) == "resource-required.") {
			std::string key = setting.first.substr(18);
			int value = 0;

			try {
				value = std::stoi(setting.second);
			}
			catch(const std::invalid_argument& e) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			catch(const std::out_of_range& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" for parameter \"" + setting.first + "\" is out of range.");
			}
			if(value <= 0) {
				throw std::runtime_error("Value for parameter \"" + setting.first + "\" must be greater than 0 but it is \"" + setting.second + "\"");
			}

			if(settings.resourcesRequired.insert(std::make_pair(key, value)).second == false) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "args") {
		    //<setting key="args" value="--propertyId=Bla --propertyFile=/etc/secret/test.pwd"/>
			if(hasArgs) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasArgs = true;
			settings.args = setting.second;
		}
		else if(setting.first == "args-flag") {
		    //<setting key="args-flag" value="override|extend|fixed"/>
			if(hasArgsFlag) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasArgsFlag = true;
			if(setting.second == "override") {
				settings.argsFlag = Flag::override;
			}
			else if(setting.second == "extend") {
				settings.argsFlag = Flag::extend;
			}
			else if(setting.second == "fixed") {
				settings.argsFlag = Flag::fixed;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "env") {
		    //<setting key="env" value="JOBID=${TASK_ID}"/>
		    //<setting key="env" value="TMP_DIR=/tmp"/>
			std::size_t pos = setting.second.find('=');
			std::string key = setting.second.substr(0, pos);
			std::string value;
			if(pos != std::string::npos) {
		        value = setting.second.substr(pos+1);
			}
			if(settings.envs.insert(std::make_pair(key, value)).second == false) {
				throw std::runtime_error("Multiple definition of key \"" + key + "\" for parameter \"env\".");
			}
		}
		else if(setting.first == "env-flag-global") {
		    //<setting key="env-flag-global" value="override|extend"/>
			if(hasEnvFlagGlobal) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasEnvFlagGlobal = true;
			if(setting.second == "override") {
				settings.envFlagGlobal = Flag::override;
			}
			else if(setting.second == "extend") {
				settings.envFlagGlobal = Flag::extend;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "env-flag") {
		    //<setting key="env-flag" value="override|extend|fixed"/>
			if(hasEnvFlag) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasEnvFlag = true;
			if(setting.second == "override") {
				settings.envFlag = Flag::override;
			}
			else if(setting.second == "extend") {
				settings.envFlag = Flag::extend;
			}
			else if(setting.second == "fixed") {
				settings.envFlag = Flag::fixed;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "cd" || setting.first == "working-directory") {
		    //<setting key="cd" value="/var/log"/>
			if(!settings.cd.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.cd = setting.second;
			if(settings.cd.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "cd-flag") {
		    //<setting key="cd-flag" value="override|fixed"/>
			if(hasCdFlag) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasCdFlag = true;
			if(setting.second == "override") {
				settings.cmdFlag = Flag::override;
			}
			else if(setting.second == "fixed") {
				settings.cmdFlag = Flag::fixed;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "cmd" || setting.first == "executable") {
		    //<setting key="cmd" value="/opt/bin/true"/>
			if(!settings.cmd.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.cmd = setting.second;
			if(settings.cmd.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "cmd-flag") {
		    //<setting key="cmd-flag" value="override|fixed"/>
			if(hasCmdFlag) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\".");
			}
			hasCmdFlag = true;
			if(setting.second == "override") {
				settings.cmdFlag = Flag::override;
			}
			else if(setting.second == "fixed") {
				settings.cmdFlag = Flag::fixed;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "yaml-file") {
		    //<setting key="yaml-file" value="./kube/kubectl"/>
			if(!settings.yamlFile.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.yamlFile = setting.second;
			if(settings.yamlFile.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "kubectl-cmd") {
		    //<setting key="kubectl-cmd" value="./kube/kubectl"/>
			if(!settings.kubectlCmd.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.kubectlCmd = setting.second;
			if(settings.kubectlCmd.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "kubectl-config") {
		    //<setting key="kubectl-config" value="./kube/dev-level-1.yaml"/>
			if(!settings.kubectlConfig.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.kubectlConfig = setting.second;
			if(settings.kubectlConfig.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "image") {
		    //<setting key="image" value="docker.io/library/hello-world:latest"/>
			if(!settings.image.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.image = setting.second;
			if(settings.image.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "namespace") {
		    //<setting key="namespace" value="batchelor4711"/>
			if(!settings.metaNamespace.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.metaNamespace = setting.second;
			if(settings.metaNamespace.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "backoff-limit") {
			if(hasBackoffLimit) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			hasBackoffLimit = true;

			try {
				settings.backoffLimit = std::stoi(setting.second);
			}
			catch(const std::invalid_argument& e) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			catch(const std::out_of_range& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" for parameter \"" + setting.first + "\" is out of range.");
			}
			if(settings.backoffLimit <= 0) {
				throw std::runtime_error("Value \"" + setting.second + "\" for parameter \"" + setting.first + "\" is out of range.");
			}
		}
		else if(setting.first == "service-account-name") {
		    //<setting key="service-account-name" value=""/>
			if(!settings.serviceAccountName.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.serviceAccountName = setting.second;
			if(settings.serviceAccountName.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "image-pull-secret") {
		    //<setting key="image-pull-secret" value="batchelor-docker-private-images"/>
		    //<setting key="image-pull-secret" value="batchelor-docker-public-images"/>
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			settings.imagePullSecrets.insert(setting.second);
		}
		else if(setting.first == "resources-requests-cpu") {
		    //<setting key="resources-requests-cpu" value="100m"/>
			if(!settings.resourcesRequestsCPU.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.resourcesRequestsCPU = setting.second;
			if(settings.resourcesRequestsCPU.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "resources-requests-memory") {
		    //<setting key="resources-requests-memory" value="100Mi"/>
			if(!settings.resourcesRequestsMemory.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.resourcesRequestsMemory = setting.second;
			if(settings.resourcesRequestsMemory.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "resources-limits-cpu") {
		    //<setting key="resources-limits-cpu" value="100m"/>
			if(!settings.resourcesLimitsCPU.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.resourcesLimitsCPU = setting.second;
			if(settings.resourcesLimitsCPU.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}
		else if(setting.first == "resources-limits-memory") {
		    //<setting key="resources-limits-memory" value="100Mi"/>
			if(!settings.resourcesLimitsMemory.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			settings.resourcesLimitsMemory = setting.second;
			if(settings.resourcesLimitsMemory.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
		}

		else if(setting.first == "volume.id") {
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			std::vector<Settings::Volume>& volumes = settings.volumes[setting.second];
			volumes.push_back(Settings::Volume());
			volumePtr = &volumes.back();
		}
		else if(setting.first == "volume.kind") {
			if(!volumePtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'volume.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!volumePtr->kind.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			volumePtr->kind = setting.second;
		}
		else if(setting.first == "volume.name") {
			if(!volumePtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'volume.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!volumePtr->name.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			volumePtr->name = setting.second;
		}
		else if(setting.first == "volume.key") {
			if(!volumePtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'volume.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!volumePtr->key.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			volumePtr->key = setting.second;
		}
		else if(setting.first == "volume.path") {
			if(!volumePtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'volume.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!volumePtr->path.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			volumePtr->path = setting.second;
		}

		else if(setting.first == "mount.id") {
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			settings.mounts.push_back(Settings::Mount());
			mountPtr = &settings.mounts.back();
			mountPtr->name = setting.second;
		}
		else if(setting.first == "mount.mount-path") {
			if(!mountPtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'mount.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!mountPtr->mountPath.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			mountPtr->mountPath = setting.second;
		}
		else if(setting.first == "mount.sub-path") {
			if(!mountPtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'mount.id' is missing");
			}
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}

			if(!mountPtr->subPath.empty()) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			mountPtr->subPath = setting.second;
		}
		else if(setting.first == "mount.read-only") {
			if(!mountPtr) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\" because parameter 'mount.id' is missing");
			}
			mountPtr->readOnly = esl::utility::String::toBool(setting.second);
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}

	if(settings.cdFlag == Flag::fixed && settings.cd.empty()) {
		throw std::runtime_error("Definition of parameter \"cd\" is required because parameter is flagged as 'fixed'");
	}

	if(settings.cmdFlag == Flag::fixed && settings.cmd.empty()) {
		throw std::runtime_error("Definition of parameter \"cmd\" is required because parameter is flagged as 'fixed'");
	}

	if(settings.kubectlCmd.empty()) {
		throw std::runtime_error("Definition of parameter \"kubectl-cmd\" is missing.");
	}
/*
	if(settings.kubectlConfig.empty()) {
		throw std::runtime_error("Definition of parameter \"kubectl-config\" is missing.");
	}
*/
	if(settings.image.empty()) {
		throw std::runtime_error("Definition of parameter \"image\" is missing.");
	}

	if(settings.resourcesRequestsCPU.empty()) {
		throw std::runtime_error("Definition of parameter \"resources-requests-cpu\" is missing.");
	}

	if(settings.resourcesRequestsMemory.empty()) {
		throw std::runtime_error("Definition of parameter \"resources-requests-memory\" is missing.");
	}

	if(settings.resourcesLimitsCPU.empty()) {
		throw std::runtime_error("Definition of parameter \"resources-limits-cpu\" is missing.");
	}

	if(settings.resourcesLimitsMemory.empty()) {
		throw std::runtime_error("Definition of parameter \"resources-limits-memory\" is missing.");
	}

	return std::unique_ptr<plugin::TaskFactory>(new TaskFactory(std::move(settings)));
}

const std::map<std::string, int>& TaskFactory::getResourcesRequired() const {
	return settings.resourcesRequired;
}

bool TaskFactory::isBusy(const std::map<std::string, int>& resourcesAvailable) {
	for(const auto& resourceRequired : settings.resourcesRequired) {
		auto resourceAvailable = resourcesAvailable.find(resourceRequired.first);
		if(resourceAvailable == resourcesAvailable.end() || resourceAvailable->second < resourceRequired.second) {
			return true;
		}
	}

	return false;
}

std::unique_ptr<plugin::Task> TaskFactory::createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const service::schemas::RunConfiguration& runConfiguration) {
	return std::unique_ptr<Task>(new Task(*this, notifyCV, notifyMutex, metrics, settings, runConfiguration));
}

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
