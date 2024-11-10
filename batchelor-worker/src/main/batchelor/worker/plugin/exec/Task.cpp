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

#include <batchelor/common/config/xml/Setting.h>
#include <batchelor/common/types/State.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/plugin/exec/Task.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/system/DefaultProcess.h>
#include <esl/system/Environment.h>
#include <esl/system/Stacktrace.h>
#include <esl/system/Signal.h>
#include <esl/utility/String.h>

#include <cstdlib>
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
: plugin::Task(factorySettings.resourcesRequired),
  taskFactory(aTaskFactoryExec),
  notifyCV(aNotifyCV),
  taskStatusMutex(notifyMutex)
{
	bool hasArgs = false;

	status.state = common::types::State::Type::running;

	for(const auto& setting : runConfiguration.settings) {
		if(setting.key == "args") {
		    //<setting key="args" value="--propertyId=Bla --propertyFile=/etc/secret/test.pwd"/>
			if(hasArgs) {
				throw std::runtime_error("Multiple definition of parameter \"" + setting.key + "\".");
			}
			hasArgs = true;

			switch(factorySettings.argsFlag) {
			case TaskFactory::Flag::override:
				settings.args = setting.value;
				break;
			case TaskFactory::Flag::extend:
				settings.args += " " + setting.value;
				break;
			case TaskFactory::Flag::fixed:
				throw esl::system::Stacktrace::add(std::runtime_error("It is not allowed to override or extend parameter \"" + setting.key + "\""));
				break;
			}
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
		    //<setting key="cd" value="/var/log"/>
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

	if(factorySettings.envFlag == TaskFactory::Flag::fixed) {
		settings.envs = factorySettings.envs;
	}
	else {
		for(const auto& env : factorySettings.envs) {
			if(factorySettings.envFlag == TaskFactory::Flag::override) {
				/* if it is allowed to override the factory settings, then we have to keep the task specific settings.
				 * So we don't override the task specific settings, but we add the map with a factory entry if it does not exists so far.
				 */
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
	settings.cmd = factorySettings.cmd;

	/* now we replace the parameters in the envs and args */
	const std::map<std::string, std::string> metrics(aMetrics.begin(), aMetrics.end());

	settings.args = common::config::xml::Setting::generalEvaluate(settings.args, true, metrics);
	for(auto& env : settings.envs) {
		env.second = common::config::xml::Setting::generalEvaluate(env.second, true, metrics);
	}
	settings.cd = common::config::xml::Setting::generalEvaluate(settings.cd, true, metrics);
	settings.cmd = common::config::xml::Setting::generalEvaluate(settings.cmd, true, metrics);
	std::string outfile = common::config::xml::Setting::generalEvaluate(factorySettings.outfile, true, metrics);
	std::string errfile = common::config::xml::Setting::generalEvaluate(factorySettings.errfile, true, metrics);


	/* check if all necessary properties are set */
	if(settings.cd.empty()) {
		throw std::runtime_error("No working directory defined to create a process.");
	}

	if(settings.cmd.empty()) {
		throw std::runtime_error("No executable defined to create a process.");
	}

	arguments = esl::system::Arguments(settings.args.empty() ? settings.cmd : settings.cmd + " " + settings.args);
	std::vector<std::pair<std::string, std::string>> envs;
	for(const auto& env : settings.envs) {
		envs.push_back(env);
	}

	process = esl::system::DefaultProcess::createNative();
	if(!process) {
		throw std::runtime_error("No process instance available to execute a process.");
	}

//	if(!envs.empty()) {
	process->setEnvironment(std::unique_ptr<esl::system::Environment>(new esl::system::Environment(std::move(envs))));
//	}

	boost::filesystem::create_directories(settings.cd);
	process->setWorkingDir(settings.cd);

#if 0
	(*process)[esl::system::FileDescriptor::getOut()] >> (settings.cd + "/out.log");
	(*process)[esl::system::FileDescriptor::getErr()] >> (settings.cd + "/err.log");
#else
	if(!outfile.empty()) {
		(*process)[esl::system::FileDescriptor::getOut()] >> outfile;
	}
	if(!errfile.empty()) {
		(*process)[esl::system::FileDescriptor::getErr()] >> errfile;
	}
#endif

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
		process->sendSignal(esl::system::Signal("interrupt"));
		process->sendSignal(esl::system::Signal("terminate"));
		process->sendSignal(esl::system::Signal("pipe"));
	}
	else {
		process->sendSignal(esl::system::Signal(signal));
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
