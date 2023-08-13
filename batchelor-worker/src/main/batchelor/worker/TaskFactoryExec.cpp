#include <batchelor/worker/Logger.h>
#include <batchelor/worker/TaskExec.h>
#include <batchelor/worker/TaskFactoryExec.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace {
Logger logger("batchelor::worker::TaskFactoryExec");
}


TaskFactoryExec::TaskFactoryExec(Settings aSettings)
: settings(std::move(aSettings))
{
	if(settings.executable == "") {
		throw esl::system::Stacktrace::add(std::runtime_error("Parameter \"executable\" is required"));
	}
	if(settings.workingDirectory == "") {
		logger.warn << "No working directory is specified.\n";
	}
}

std::unique_ptr<TaskFactory> TaskFactoryExec::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;

	for(const auto& setting : aSettings) {
		if(setting.first == "maximum-jobs-running") {
			if(settings.maximumJobsRunning != 0) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"maximum-jobs-running\""));
			}
			int maximumJobsRunning = std::stoi(setting.second);
			if(maximumJobsRunning < 0) {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"" + setting.second + "\" for parameter \"maximum-jobs-running\""));
			}
			settings.maximumJobsRunning = static_cast<std::size_t>(maximumJobsRunning);
		}
		else if(setting.first == "cmd" || setting.first == "executable") {
			if(settings.executable != "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"cmd\""));
			}
			settings.executable = setting.second;
			if(settings.executable == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"cmd\""));
			}
		}
		else if(setting.first == "cd" || setting.first == "working-directory") {
			if(settings.workingDirectory != "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"working-directory\""));
			}
			settings.workingDirectory = setting.second;
			if(settings.workingDirectory == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"working-directory\""));
			}
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}

	return std::unique_ptr<TaskFactory>(new TaskFactoryExec(std::move(settings)));
}

bool TaskFactoryExec::isBusy() {
	if(settings.maximumJobsRunning > 0 && processesRunning >= settings.maximumJobsRunning) {
		return true;
	}
	return false;
}

std::unique_ptr<Task> TaskFactoryExec::createTask(std::condition_variable& notifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& aSettings) {
	++processesRunning;
	return std::unique_ptr<Task>(new TaskExec(*this, notifyCV, notifyMutex, aSettings, settings.executable, settings.workingDirectory));
}

void TaskFactoryExec::releaseProcess() {
	--processesRunning;
}

} /* namespace worker */
} /* namespace batchelor */
