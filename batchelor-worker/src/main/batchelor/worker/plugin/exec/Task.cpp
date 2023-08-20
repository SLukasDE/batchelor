#include <batchelor/common/types/State.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/exec/Task.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/system/Environment.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/Signal.h>

#include <boost/filesystem.hpp>

#include <mutex>
#include <stdexcept>

namespace batchelor {
namespace worker {
namespace plugin {
namespace exec {
namespace {
Logger logger("batchelor::worker::plugin::exec::Task");
}

Task::Task(TaskFactory& aTaskFactoryExec, std::condition_variable& aNotifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& aMetrics, const TaskFactory::Settings& factorySettings, const service::schemas::RunConfiguration& runConfiguration)
: taskFactory(aTaskFactoryExec),
  notifyCV(aNotifyCV),
  taskStatusMutex(notifyMutex)
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
		else if(setting.key == "cd" || setting.key == "working-directory") {
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

	arguments = esl::system::Arguments(settings.args.empty() ? settings.cmd : settings.cmd + " " + settings.args);
	std::vector<std::pair<std::string, std::string>> envs;
	for(const auto& env : settings.envs) {
		envs.push_back(env);
	}

	process = esl::plugin::Registry::get().create<esl::system::Process>("eslx/system/Process", {});
	if(!process) {
		throw std::runtime_error("No process instance available to execute a process.");
	}

	if(!envs.empty()) {
		process->setEnvironment(std::unique_ptr<esl::system::Environment>(new esl::system::Environment(std::move(envs))));
	}

	if(!settings.cd.empty()) {
		//throw esl::system::Stacktrace::add(std::runtime_error("Parameter \"working-directory\" is required"));
		boost::filesystem::create_directories(settings.cd);
		process->setWorkingDir(settings.cd);
	}

	(*process)[esl::system::FileDescriptor::getOut()] >> boost::filesystem::path(settings.cd + "/out.log");
	(*process)[esl::system::FileDescriptor::getErr()] >> boost::filesystem::path(settings.cd + "/err.log");

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
		process->sendSignal(esl::utility::Signal("interrupt"));
		process->sendSignal(esl::utility::Signal("terminate"));
		process->sendSignal(esl::utility::Signal("pipe"));
	}
	else {
		process->sendSignal(esl::utility::Signal(signal));
	}
}

void Task::run() {
	try {
		logger.debug << "execute ...\n";
		auto returnCode = process->execute(arguments);

		{
			std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);
			status.state = common::types::State::Type::done;
			status.returnCode = returnCode;
			taskFactory.releaseProcess();
		}

		logger.debug << "execution done, rc=" << returnCode << "\n";
	}
	catch(const std::exception& e) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		status.state = common::types::State::Type::signaled;
		status.message = e.what();

		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		status.state = common::types::State::Type::signaled;
		status.message = "Execution failed because of unknown exception.";

		logger.warn << "Execution failed because of unknown exception.\n";
	}

	notifyCV.notify_all();
}

} /* namespace exec */
} /* namespace plugin */
} /* namespace worker */
} /* namespace batchelor */
