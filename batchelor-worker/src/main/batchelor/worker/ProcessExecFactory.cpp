#include <batchelor/worker/Logger.h>
#include <batchelor/worker/ProcessExecFactory.h>
#include <batchelor/worker/ProcessExec.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace worker {
namespace {
Logger logger("batchelor::worker::ProcessExecFactory");
}


ProcessExecFactory::ProcessExecFactory(Settings aSettings)
: settings(std::move(aSettings))
{
	if(settings.executable == "") {
		throw esl::system::Stacktrace::add(std::runtime_error("Parameter \"executable\" is required"));
	}
	if(settings.workingDirectory == "") {
		logger.warn << "No working directory is specified.\n";
	}
}

std::unique_ptr<ProcessFactory> ProcessExecFactory::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;

	for(const auto& setting : aSettings) {
		if(setting.first == "executable") {
			if(settings.executable != "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"executable\""));
			}
			settings.executable = setting.second;
			if(settings.executable == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"executable\""));
			}
		}
		else if(setting.first == "working-directory") {
			if(settings.workingDirectory != "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"working-directory\""));
			}
			settings.workingDirectory = setting.second;
			if(settings.workingDirectory == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"working-directory\""));
			}
		}
		else if(setting.first == "maximum-jobs-running") {
			if(settings.maximumJobsRunning != 0) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"maximum-jobs-running\""));
			}
			int maximumJobsRunning = std::stoi(setting.second);
			if(maximumJobsRunning < 0) {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"" + setting.second + "\" for parameter \"maximum-jobs-running\""));
			}
			settings.maximumJobsRunning = static_cast<std::size_t>(maximumJobsRunning);
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}

	return std::unique_ptr<ProcessFactory>(new ProcessExecFactory(std::move(settings)));
}

std::unique_ptr<esl::object::Object> ProcessExecFactory::createObject(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Object>(create(settings).release());
}

bool ProcessExecFactory::isBusy(Procedure& procedure) {
	if(settings.maximumJobsRunning > 0 && processesRunning >= settings.maximumJobsRunning) {
		return true;
	}
	return false;
}

std::unique_ptr<Process> ProcessExecFactory::ProcessExecFactory::createProcess(Procedure& procedure, const std::vector<std::pair<std::string, std::string>>& aSettings) {
	++processesRunning;
	return std::unique_ptr<Process>(new ProcessExec(*this, procedure, aSettings, settings.workingDirectory, settings.workingDirectory));
}

void ProcessExecFactory::releaseProcess() {
	--processesRunning;
}

} /* namespace worker */
} /* namespace batchelor */
