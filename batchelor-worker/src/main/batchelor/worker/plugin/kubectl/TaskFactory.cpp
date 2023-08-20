#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/kubectl/Task.h>
#include <batchelor/worker/plugin/kubectl/TaskFactory.h>

#include <esl/system/Stacktrace.h>

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

	bool hasMetricsPolicy = false;
	bool hasArgs = false;
	bool hasArgsFlag = false;
	bool hasEnvFlagGlobal = false;
	bool hasEnvFlag = false;
	bool hasCdFlag = false;
	bool hasCmdFlag = false;

	for(const auto& setting : aSettings) {
		if(setting.first == "maximum-tasks-running") {
		    //<setting key="maximum-tasks-running" value="3"/>
			if(settings.maximumTasksRunning != 0) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.first + "\"");
			}
			int maximumJobsRunning = 0;
			try {
				maximumJobsRunning = std::stoi(setting.second);
			}
			catch(const std::invalid_argument& e) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter \"" + setting.first + "\"");
			}
			catch(const std::out_of_range& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" for parameter \"" + setting.first + "\" is out of range.");
			}
			if(maximumJobsRunning <= 0) {
				throw std::runtime_error("Value for parameter \"" + setting.first + "\" must be greater than 0 but it is \"" + setting.second + "\"");
			}
			settings.maximumTasksRunning = static_cast<std::size_t>(maximumJobsRunning);
		}
		else if(setting.first == "metrics-policy") {
			if(hasMetricsPolicy) {
				throw std::runtime_error("Multiple definition of attribute \"metrics-policy\".");
			}

			hasMetricsPolicy = true;

			if(setting.second == "allow") {
				settings.metricsPolicy = MetricsPolicy::allow;
			}
			else if(setting.second == "deny") {
				settings.metricsPolicy = MetricsPolicy::deny;
			}
			else {
				throw std::runtime_error("Value \"" + setting.second + "\" of attribute 'metrics-policy' is invalid");
			}
		}
		else if(setting.first == "metric") {
			settings.metrics.insert(setting.second);
		}
		else if(setting.first == "args") {
		    //<setting key="args" value="--propertyId=Bla --propertyFile=/wxx/secret/property.cfg"/>
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
		    //<setting key="cd" value="/wxx/app/rose/log"/>
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
		    //<setting key="cmd" value="/opt/bin/bestoptxl-calculation"/>
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

	return std::unique_ptr<plugin::TaskFactory>(new TaskFactory(std::move(settings)));
}

bool TaskFactory::isBusy() {
	if(settings.maximumTasksRunning > 0 && tasksRunning >= settings.maximumTasksRunning) {
		return true;
	}
	return false;
}

std::unique_ptr<plugin::Task> TaskFactory::createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& metrics, const service::schemas::RunConfiguration& runConfiguration) {
	++tasksRunning;
	return std::unique_ptr<Task>(new Task(*this, notifyCV, notifyMutex, metrics, settings, runConfiguration));
}

void TaskFactory::releaseProcess() {
	--tasksRunning;
}

} /* namespace kubectl */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
