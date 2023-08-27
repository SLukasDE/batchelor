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
		struct Volume {
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
		std::string kubectlCmd = "./kube/kubectl";
		std::string kubectlConfig = "./kube/dev-level-1.yaml";

		std::string image = "tsco-docker-private-images.artifactory.rewe.local/k8s-batch:1.0.0-alpha.28";
		std::string cd;
		std::string cmd;
		std::string args;
		std::map<std::string, std::string> envs;

		std::set<std::string> imagePullSecrets = {{"tsco-docker-private-images"}, {"tsco-docker-public-images"}};
		std::string resourcesRequestsCPU = "100m";
		std::string resourcesRequestsMemory = "100Mi";
		std::string resourcesLimitsCPU = "100m";
		std::string resourcesLimitsMemory = "100Mi";
		std::map<std::string, std::vector<Volume>> volumes = { {{"secret-mounts"}, {{"secret", "k8s-batch", "esl-logger", "logger.xml"}, {"secret", "k8s-batch", "rose-db", "rose-db.xml"}}} };
		std::vector<Mount> mounts = { {"/opt/etc/logger.xml", "secret-mounts", "logger.xml", true}, {"/opt/etc/rose-db.xml", "secret-mounts", "rose-db.xml", true}};

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

	std::vector<std::pair<std::string, std::string>> metrics;
	Settings settings;

	std::thread thread;

	void run();

	std::unique_ptr<esl::system::Process> getProcess() const;
	std::string getCmd() const noexcept;
	Status runBatch() const noexcept;
	void sendCancel() const noexcept;
	std::string getDeploymentYAML() const noexcept;
	Status getPodStatus();
};

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */

#endif /* BATCHELOR_WORKER_PLUGIN_KUBECTL_TASK_H_ */
