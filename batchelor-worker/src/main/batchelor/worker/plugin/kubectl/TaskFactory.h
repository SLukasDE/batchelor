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

#ifndef BATCHELOR_WORKER_PLUGIN_KUBECTL_TASKFACTORY_H_
#define BATCHELOR_WORKER_PLUGIN_KUBECTL_TASKFACTORY_H_

#include <batchelor/service/schemas/RunConfiguration.h>

#include <batchelor/worker/plugin/Task.h>
#include <batchelor/worker/plugin/TaskFactory.h>

#include <esl/object/Object.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace worker {
namespace plugin {
namespace kubectl {

class TaskFactory : public plugin::TaskFactory {
public:
	enum class MetricsPolicy {
		allow, deny
	};
	enum class Flag {
		override, extend, fixed
	};
	struct Settings {
		struct Volume {
			Volume() = default;
			Volume(std::string aKind, std::string aName, std::string aKey, std::string aPath)
			: kind(aKind),
			  name(aName),
			  key(aKey),
			  path(aPath)
			{ }
			std::string kind; // "secret", "configMap", ...
			std::string name;
			std::string key;
			std::string path;
		};
		struct Mount {
			Mount() = default;
			Mount(std::string aMountPath, std::string aName, std::string aSubPath, bool aReadOnly)
			: mountPath(aMountPath),
			  name(aName),
			  subPath(aSubPath),
			  readOnly(aReadOnly)
			{ }
			std::string mountPath;
			std::string name; // this is the key of volumes map
			std::string subPath; // this is the path of the volume entry in volue map
			bool readOnly = true;
		};

		std::map<std::string, int> resourcesRequired;

		std::string args;
		Flag argsFlag = Flag::fixed; // override|extend|fixed

		std::map<std::string, std::string> envs;
		Flag envFlagGlobal = Flag::extend; // override|extend
		Flag envFlag = Flag::extend; // override|extend|fixed

		std::string cd;
		Flag cdFlag = Flag::fixed; // override|fixed

		std::string cmd;
		Flag cmdFlag = Flag::fixed; // override|fixed

		std::string yamlFile;
		std::string kubectlCmd;
		std::string kubectlConfig;
		std::string image;
		std::string serviceAccountName;
		std::string metaNamespace;
		int backoffLimit = 0;
		std::set<std::string> imagePullSecrets;
		std::string resourcesRequestsCPU;
		std::string resourcesRequestsMemory;
		std::string resourcesLimitsCPU;
		std::string resourcesLimitsMemory;
		std::map<std::string, std::vector<Volume>> volumes;
		std::vector<Mount> mounts;
	};

	TaskFactory(Settings settings);

	/* settings:
	 * - arguments and
	 * - task specific arguments-flag, tells if task specific arguments are
	 *   - overwriting these arguments or
	 *   - extending these arguments or
	 *   - not allowed
	 * - environment variables and
	 * - global environment-flag, tells if these environments are
	 *   - overwriting system environments or
	 *   - extending system environments and
	 * - task specific environment-flag, tells if these environments are
	 *   - overwriting these environments or
	 *   - extending these environments or
	 *   - not allowed
	 * - working dir,
	 * - executable, ...
	 * e.g.:
	 * - settings[ 1] = { 'args' ;              '--propertyId=Bla --propertyFile=/etc/secret/test.pwd' }
	 * - settings[ 2] = { 'args-flag' ;         'override|extend|fixed' }
	 * - settings[ 3] = { 'env' ;               'DISPLAY=0' }
	 * - settings[ 4] = { 'env' ;               'TMP_DIR=/tmp' }
	 * - settings[ 5] = { 'env-flag-global' ;   'override|extend' }
	 * - settings[ 6] = { 'env-flag' ;          'override|extend|fixed' }
	 * - settings[ 7] = { 'cd' ;                '/var/log' }
	 * - settings[ 8] = { 'cd-flag' ;           'override|fixed' }
	 * - settings[ 9] = { 'cmd' ;               '/opt/bin/true' }
	 * - settings[10] = { 'cmd-flag' ;          'override|fixed' }
	 */
	static std::unique_ptr<plugin::TaskFactory> create(const std::vector<std::pair<std::string, std::string>>& settings);

	const std::map<std::string, int>& getResourcesRequired() const override;
	bool isBusy(const std::map<std::string, int>& resourcesAvailable) override;

	/* settings:
	 * - arguments,
	 * - environment variables,
	 * - working dir,
	 * - executable, ...
	 * e.g.:
	 * - settings[0] = { 'args' ; '--propertyId=Bla --propertyFile=/etc/secret/test.pwd' }
	 * - settings[1] = { 'env'  ; 'DISPLAY=0' }
	 * - settings[2] = { 'env' ;  'TMP_DIR=/tmp' }
	 * - settings[3] = { 'cd' ;   '/var/log' }
	 * - settings[4] = { 'cmd' ;  '/opt/bin/true' }
	 */
	std::unique_ptr<plugin::Task> createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const service::schemas::RunConfiguration& runConfiguration) override;

private:
	Settings settings;
};

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_KUBECTL_TASKFACTORY_H_ */
