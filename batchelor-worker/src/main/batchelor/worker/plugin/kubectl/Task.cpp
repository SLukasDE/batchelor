#include <batchelor/common/types/State.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/kubectl/Task.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/io/input/String.h>
#include <esl/io/output/Memory.h>
#include <esl/io/Consumer.h>
#include <esl/system/Environment.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/String.h>

#include <boost/filesystem.hpp>

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

class MyConsumer : public esl::io::Consumer {
public:
	MyConsumer() = default;

	bool consume(esl::io::Reader& reader) override {
		std::cout << "Consume...\n";
		char buffer[4096];
		std::size_t count = reader.read(buffer, sizeof(buffer));

		if(count == esl::io::Reader::npos) {
			return false;
		}

		if(count > 4096) {
			logger.error << "CONSUMED reader returned " << count << " bytes\n";
		}
		else {
			str << std::string(buffer, count);
		}

		return true;
	}

	std::stringstream& getStringStream() noexcept {
		return str;
	}

private:
	std::stringstream str;
};
}

Task::Task(TaskFactory& aTaskFactoryExec, std::condition_variable& aNotifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& aMetrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration)
: taskId(runConfiguration.taskId),
  eventType(runConfiguration.eventType),
  taskFactory(aTaskFactoryExec),
  notifyCV(aNotifyCV),
  taskStatusMutex(notifyMutex),
  metrics(aMetrics)
{
	bool hasArgs = false;

	status.state = common::types::State::Type::running;

	for(const auto& setting : runConfiguration.settings) {
		if(setting.key == "args") {
		    //<setting key="args" value="--propertyId=Bla --propertyFile=/wxx/secret/property.cfg"/>
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
		    //<setting key="env" value="JOBID=${TASK_ID}"/>
		    //<setting key="env" value="TMP_DIR=/tmp"/>
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
		    //<setting key="cd" value="/wxx/app/rose/log"/>
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
		    //<setting key="cmd" value="/opt/bin/bestoptxl-calculation"/>
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
		for(const auto env : factorySettings.envs) {
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
	if(settings.cd.empty()) {
		throw std::runtime_error("No working directory defined to create a process.");
	}

	if(settings.cmd.empty()) {
		settings.cmd = factorySettings.cmd;
	}
	if(settings.cmd.empty()) {
		throw std::runtime_error("No executable defined to create a process.");
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
		if(!runBatch()) {
			sendCancel();

			std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);
			status.state = common::types::State::Type::signaled;
			status.returnCode = 1;
			status.message = "Applying batch config failed";
			taskFactory.releaseProcess();
			return;
		}

		PodStatus podStatus;
		while(podStatus.state == PodState::active) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			podStatus = getPodStatus();
		}

		{
			std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);
			if(podStatus.state == PodState::zombie) {
				status.state = common::types::State::Type::zombie;
				status.returnCode = 1;
			}
			else {
				status.state = common::types::State::Type::done;
				status.returnCode = podStatus.state == PodState::succeeded ? 0 : 1;
			}
			status.message = podStatus.line;
			taskFactory.releaseProcess();
		}
std::cout << "execution done, status: " << podStatus.line << "\n";
		logger.debug << "execution done, status: " << podStatus.line << "\n";
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

std::unique_ptr<esl::system::Process> Task::getProcess() const {
	std::unique_ptr<esl::system::Process> process;
	process = esl::plugin::Registry::get().create<esl::system::Process>("eslx/system/Process", {});
	if(!process) {
		throw std::runtime_error("No process instance available to execute a process.");
	}
	return process;
}

std::string Task::getCmd() const noexcept {
	std::string kubectlCmd = settings.kubectlCmd;
	if(!settings.kubectlConfig.empty()) {
		kubectlCmd += " --kubeconfig " + settings.kubectlConfig;
	}
	return kubectlCmd;
}

bool Task::runBatch() const noexcept {
	bool success = false;
	try {
		std::string deploymentYAML = getDeploymentYAML();
		esl::io::output::Memory deploymentYamlProducer(deploymentYAML.data(), deploymentYAML.size());

		std::unique_ptr<esl::system::Process> process = getProcess();

		(*process)[esl::system::FileDescriptor::getIn()] << esl::io::Output(deploymentYamlProducer);
		//(*process)[esl::system::FileDescriptor::getOut()] >> esl::io::Input(kubectlOutput);
		//(*process)[esl::system::FileDescriptor::getErr()] >> boost::filesystem::path(settings.cd + "/err.log");

		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " apply -f -"));
		success = returnCode == 0;
		logger.debug << "execution done, rc=" << returnCode << "\n";
	}
	catch(const std::exception& e) {
		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		logger.warn << "Execution failed because of unknown exception.\n";
	}
	return success;
}

void Task::sendCancel() const noexcept {
	try {
		std::unique_ptr<esl::system::Process> process = getProcess();
		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " delete job " + taskId));
		taskCanceled = true;
std::cout << "canceled task id " << taskId << " and received return code " << returnCode << "\n";
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
	<< "  name: " << taskId
	<< "spec:\n"
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
			deploymentYaml
			<< "        - name: " << volume.first << "\n";
			/* todo: put all entry together with same kind+name.
			 * there should not be multiple entries with same name but different kind.
			 * If there are multiple entries with same name (+kind), collect all items together.
			 */

			for(const auto& entry : volume.second) {
				deploymentYaml
				<< "          " << entry.kind << ":\n";
				if(entry.kind == "secret") {
					deploymentYaml
					<< "            name: " << entry.name << "\n";
				}
				else {
					deploymentYaml
					<< "            secretName: " << entry.name << "\n";
				}
				deploymentYaml
				<< "            items:\n"
				<< "              - key: " << entry.key << "\n"
				<< "                path: " << entry.path << "\n";

			}

		}
	}
	deploymentYaml
	<< "      containers:\n"
	<< "        - name: " << taskId << "\n"
	<< "          image: \"" << settings.image << "\"\n"
	<< "          imagePullPolicy: Always\n"
	<< "          command: [\"" << settings.cmd << "\"]\n"
	<< "          args:\n";
	{
		esl::system::Arguments arguments(settings.args);
		std::size_t argc = arguments.getArgc();
		char** argv = arguments.getArgv();
		for(std::size_t i=0; i<argc; ++i) {
			deploymentYaml << "            - \"" << argv[i] << "\"\n";
		}
	}

	deploymentYaml
	<< "          resources:\n"
	<< "            requests:\n"
	<< "              cpu: \"" << settings.resourcesRequestsCPU << "\"\n"
	<< "              memory: \"" << settings.resourcesRequestsMemory << "\"\n"
	<< "            limits:\n"
	<< "              cpu: \"" << settings.resourcesLimitsCPU << "\"\n"
	<< "              memory: \"" << settings.resourcesLimitsMemory << "\"\n"
	<< "          volumeMounts:\n"
	<< "\n";
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
std::cout << deploymentYaml.str();

	return deploymentYaml.str();
}

Task::PodStatus Task::getPodStatus() {
	try {
std::cout << "Run: " << getCmd() << " describe job " << taskId << "\n";
		std::unique_ptr<esl::system::Process> process = getProcess();

		//std::string emptyString = "Hello";
		//esl::io::output::Memory emptyInput(emptyString.data(), emptyString.size());
		//(*process)[esl::system::FileDescriptor::getIn()] << esl::io::Output(emptyInput);
		//(*process)[esl::system::FileDescriptor::getIn()] << esl::io::Output();

		MyConsumer kubectlOutput;
		//esl::io::input::String kubectlOutput;
		(*process)[esl::system::FileDescriptor::getOut()] >> esl::io::Input(kubectlOutput);
		//(*process)[esl::system::FileDescriptor::getOut()] >> boost::filesystem::path("out.log");
		//(*process)[esl::system::FileDescriptor::getErr()] >> boost::filesystem::path("err.log");
		auto returnCode = process->execute(esl::system::Arguments(getCmd() + " describe job " + taskId));
		if(returnCode != 0) {
			Task::PodStatus podStatus;

			if(taskCanceled) {
				podStatus.state = PodState::signaled;
				podStatus.line = "Cancel executed";
			}
			else {
				podStatus.state = PodState::zombie;
				podStatus.line = "Could not get description of job";
			}

			return podStatus;
		}

		std::stringstream& ss(kubectlOutput.getStringStream());
		std::string line;
		while(std::getline(ss, line, '\n')) {
			line = esl::utility::String::ltrim(line);

			// Pods Statuses:    1 Active / 0 Succeeded / 0 Failed
			std::size_t pos = line.find(':');
			if(pos == std::string::npos || line.substr(0, pos) != "Pods Statuses") {
				continue;
			}

			Task::PodStatus podStatus;
			podStatus.line = esl::utility::String::ltrim(line.substr(pos+1));

			std::vector<std::string> values = esl::utility::String::split(podStatus.line, ' ', true);
			// 1 Active / 0 Succeeded / 0 Failed
			if(values.size() != 8) {
				podStatus.state = PodState::zombie;
				podStatus.line = "Invalid pods status format. It should has 8 chanks, but there are " + std::to_string(values.size()) + " chunks: " + podStatus.line;
			}
			try {
				if(std::stoi(values[3]) > 0) {
					podStatus.state = PodState::succeeded;
				}
				else if(std::stoi(values[6]) > 0) {
					podStatus.state = PodState::failed;
				}
				else {
					podStatus.state = PodState::active;
				}
			}
			catch(...) {
				podStatus.state = PodState::zombie;
				podStatus.line = "conversion failed: " + podStatus.line;
			}
			return podStatus;
		}
	}
	catch(...) {
		logger.warn << "Execution failed.\n";

		Task::PodStatus podStatus;

		podStatus.state = PodState::zombie;
		podStatus.line = "Could not get description of job because process thew an exception";

		return podStatus;
	}

	Task::PodStatus podStatus;

	podStatus.state = PodState::zombie;
	podStatus.line = "Could not get status of pod because there is no such line in description";

	return podStatus;
}

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
