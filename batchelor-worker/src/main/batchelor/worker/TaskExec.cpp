#include <batchelor/common/types/State.h>

#include <batchelor/worker/Logger.h>
#include <batchelor/worker/TaskExec.h>

#include <batchelor/service/schemas/Setting.h>

#include <esl/system/Environment.h>
#include <esl/system/Stacktrace.h>

#include <boost/filesystem.hpp>

#include <mutex>
#include <stdexcept>

namespace batchelor {
namespace worker {
namespace {
Logger logger("batchelor::worker::TaskExec");
}

TaskExec::TaskExec(TaskFactoryExec& aTaskFactoryExec, std::condition_variable& aNotifyCV, std::mutex& notifyMutex, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& aExecutable, const std::string& aWorkingDirectory)
: taskFactoryExec(aTaskFactoryExec),
  notifyCV(aNotifyCV),
  taskStatusMutex(notifyMutex),
  executable(aExecutable),
  workingDirectory(aWorkingDirectory)
{
	std::vector<std::string> args;
	std::vector<std::pair<std::string, std::string>> envs;
	taskStatus.state = common::types::State::Type::running;

	args.push_back(executable);

	for(const auto& setting : settings) {
		if(setting.first == "arg") {
			args.push_back(setting.second);
		}
		else if(setting.first.rfind("env:", 0) == 0) {
			if(setting.first.size() <= 4) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid process-exec key \"" + setting.first + "\""));
			}
			envs.push_back(std::make_pair(setting.first.substr(3), setting.second));
		}
		else {
			throw esl::system::Stacktrace::add(std::runtime_error("Unknown process-exec key \"" + setting.first + "\""));
		}
	}

	std::vector<const char*> cArgs;
	for(std::size_t i = 0; i < args.size(); ++i) {
		//char* c = const_cast<char*>(args[i].c_str());
		cArgs.push_back(args[i].c_str());
	}
	arguments = esl::system::Arguments(cArgs.size(), &cArgs[0]);

	process = esl::plugin::Registry::get().create<esl::system::Process>("eslx/system/Process", {});
	if(!process) {
		throw esl::system::Stacktrace::add(std::runtime_error("No process instance available to execute a process."));
	}

	if(!envs.empty()) {
		process->setEnvironment(std::unique_ptr<esl::system::Environment>(new esl::system::Environment(std::move(envs))));
	}

	if(!workingDirectory.empty()) {
		//throw esl::system::Stacktrace::add(std::runtime_error("Parameter \"working-directory\" is required"));
		boost::filesystem::create_directories(workingDirectory);
		process->setWorkingDir(workingDirectory);
	}

	(*process)[esl::system::FileDescriptor::getOut()] >> boost::filesystem::path(workingDirectory + "/out.log");
	(*process)[esl::system::FileDescriptor::getErr()] >> boost::filesystem::path(workingDirectory + "/err.log");

	thread = std::thread(&TaskExec::run, this);
}

TaskExec::~TaskExec() {
	thread.join();
	taskFactoryExec.releaseProcess();
}

TaskStatus TaskExec::getTaskStatus() const {
	return taskStatus;
}

void TaskExec::sendSignal(const esl::utility::Signal& signal) {
	process->sendSignal(signal);
}

void TaskExec::run() {
	try {
		logger.debug << "execute ...\n";
		auto returnCode = process->execute(arguments);

		{
			std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);
			taskStatus.state = common::types::State::Type::done;
			taskStatus.returnCode = returnCode;
		}

		logger.debug << "execution done, rc=" << returnCode << "\n";
	}
	catch(const std::exception& e) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		taskStatus.state = common::types::State::Type::signaled;
		taskStatus.message = e.what();

		logger.warn << "Execution failed because of exception: \"" << e.what() << "\"\n";
	}
	catch(...) {
		std::lock_guard<std::mutex> taskStatusLock(taskStatusMutex);

		taskStatus.state = common::types::State::Type::signaled;
		taskStatus.message = "Execution failed because of unknown exception.";

		logger.warn << "Execution failed because of unknown exception.\n";
	}

	notifyCV.notify_all();
}

} /* namespace worker */
} /* namespace batchelor */
