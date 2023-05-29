#include <batchelor/worker/Logger.h>
#include <batchelor/worker/Procedure.h>
#include <batchelor/worker/ProcessZombie.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/FetchRequest.h>
#include <batchelor/service/schemas/FetchResponse.h>
#include <batchelor/service/schemas/JobStatusHead.h>
#include <batchelor/service/schemas/JobStatusWorker.h>
#include <batchelor/service/schemas/RunConfiguration.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>
#include <batchelor/service/schemas/Setting.h>
#include <batchelor/service/schemas/Signal.h>

#include <esl/system/Stacktrace.h>

#include <chrono>
#include <stdexcept>


namespace batchelor {
namespace worker {
namespace {
Logger logger("batchelor::worker::Procedure");

static const unsigned int intervalSeconds = 5;
}

Procedure::InitializedSettings::InitializedSettings(const Settings& settings, esl::object::Context& context)
{
	for(const auto& id : settings.connectionFactoryIds) {
		connectionFactories.push_back(std::ref(context.getObject<esl::com::http::client::ConnectionFactory>(id)));
	}

	for(const auto& id : settings.processFactoryIds) {
		processFactories.emplace(id, std::ref(context.getObject<ProcessFactory>(id)));
	}
}

Procedure::Procedure(Settings aSettings)
: settings(std::move(aSettings))
{
	if(settings.connectionFactoryIds.empty()) {
		throw esl::system::Stacktrace::add(std::runtime_error("At least one parameter \"connection-factory\" is required."));
	}

	if(settings.processFactoryIds.empty()) {
		throw esl::system::Stacktrace::add(std::runtime_error("No process factories specified"));
	}
}

std::unique_ptr<esl::processing::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& aSettings) {
	Settings settings;

	for(const auto& setting : aSettings) {
		if(setting.first == "http-connection-factory-id") {
			if(settings.connectionFactoryIds.insert(setting.second).second == false) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of invalid value \"" + setting.second + "\" for parameter \"http-connection-factory-id\"."));
			}
			if(setting.second == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"http-connection-factory-id\""));
			}
		}
		else if(setting.first.rfind("id:", 0) == 0) {
			if(setting.first.size() <= 3) {
				throw esl::system::Stacktrace::add(std::runtime_error("Invalid key \"" + setting.first + "\""));
			}
			if(settings.ids.insert(std::make_pair(setting.first.substr(3), setting.second)).second == false) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"" + setting.first + "\""));
			}
			if(setting.second == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"" + setting.first + "\""));
			}
		}
		else if(setting.first == "process-factory-id") {
			if(settings.processFactoryIds.insert(setting.second).second == false) {
				throw esl::system::Stacktrace::add(std::runtime_error("Multiple definition of parameter \"process-factory-id\""));
			}
			if(setting.second == "") {
				throw esl::system::Stacktrace::add(std::runtime_error("Definition of invalid value \"\" for parameter \"process-factory-id\""));
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

	return std::unique_ptr<esl::processing::Procedure>(new Procedure(std::move(settings)));
}

void Procedure::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(settings, context));
}

void Procedure::procedureRun(esl::object::Context& context) {
	if(!initializedSettings) {
		throw esl::system::Stacktrace::add(std::runtime_error("Initialization failed"));
	}

	std::unique_lock<std::mutex> eventLock(eventMutex);

	logger.info << "Get old running jobs from head and register them as zombie jobs.\n";

	while(!eventStop) {
		if(processRequest([this](service::Service& service) { markJobsAsZombie(service); } )) {
			break;
		}

		eventCondVar.wait_for(eventLock, std::chrono::seconds(intervalSeconds), [this]{return eventStop.load();});
		//std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
	}

	logger.info << "Start event loop.\n";

	while(!eventStop) {
		processRequest([this](service::Service& service) { processEvent(service, eventStop); } );

		eventCondVar.wait_for(eventLock, std::chrono::seconds(intervalSeconds), [this]{return event || eventStop;});
		event = false;
	}

	logger.info << "Finished event loop\n";

	/* now terminate running processes. */

	/* Set eventStop back to false to figure out if the worker receives a second terminate signal.
	 * if eventStop is false, then sent signals 'interrupt', 'terminate' and 'pipe' to all running processes.
	 * if eventStop is true, then sent signal 'kill' - this happens if the worker received a 2nd terminate signal or if 'intervalSeconds' elapsed and processes are still running.
	 */
	eventStop = false;
	const auto stopTime = std::chrono::system_clock::now();

	while(!processByJobId.empty()) {
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(stopTime - std::chrono::system_clock::now());
		if(seconds.count() >= intervalSeconds) {
			eventStop = true;
		}

		if(eventStop) {
			sendSignalToProcessesRunning(esl::utility::SignalType::interrupt);
			sendSignalToProcessesRunning(esl::utility::SignalType::terminate);
			sendSignalToProcessesRunning(esl::utility::SignalType::pipe);
		}
		else {
			sendSignalToProcessesRunning(esl::utility::SignalType::kill);
		}

		processRequest([this](service::Service& service) { processEvent(service, true); } );

		eventCondVar.wait_for(eventLock, std::chrono::seconds(intervalSeconds), [this]{return event;});
		event = false;
	}
}

void Procedure::procedureCancel() {
	eventStop = true;
	eventCondVar.notify_one();
}

void Procedure::signalEvent() {
	{
		std::lock_guard<std::mutex> runningLock(eventMutex);
		event = true;
	}

	eventCondVar.notify_one();
}

bool Procedure::processRequest(std::function<void(service::Service&)> processor) {
	std::unique_ptr<esl::com::http::client::Connection> connection = createHttpClientConnection();
	service::client::Service service(*connection);

	try {
		processor(service);
	}
	catch(const std::exception& e) {
		rotateHttpClientConnection();

		logger.error << "Exception : std::exception\n";
		logger.error << "Message   : " << e.what() << "\n";

		return false;
	}
	catch(...) {
		rotateHttpClientConnection();

		logger.error << "Unknown exception occurred.\n";

		return false;
	}

	return true;
}

void Procedure::markJobsAsZombie(service::Service& service) {
	logger.info << "Loading additional running jobs:\n";
	std::vector<service::schemas::JobStatusHead> jobsRunning = service.getJobs(common::types::State::toString(common::types::State::Type::running));
	logger.info << "-> " << jobsRunning.size() << " jobs loaded\n";

	std::size_t markedJobs = 0;
	for(const auto& jobRunning : jobsRunning) {
		bool found = true;
		for(const auto& id : jobRunning.ids) {
			auto iter = settings.ids.find(id.key);
			if(iter == settings.ids.end() || id.value != iter->second) {
				found = false;
				break;
			}
		}

		if(found) {
			processByJobId[jobRunning.jobId].reset(new ProcessZombie());
			++markedJobs;
		}
	}
	logger.info << "-> Prepared " << markedJobs << " jobs to mark as zombie.\n";
}

void Procedure::processEvent(service::Service& service, bool stopping) {
	logger.debug << "Event...\n";

	/* send metrics and available process ids and fetch new jobs */
	logger.debug << "Fetch job...\n";


	service::schemas::FetchRequest fetchRequest;
	std::size_t processesRunningCurrent = calculateProcessesRunning();
	//unpublishedStopStatus |= processesRunningCurrent > 0;

	/* construct available batch id's */
	if(!stopping && (settings.maximumJobsRunning == 0 || processesRunningCurrent < settings.maximumJobsRunning)) {
		for(const auto& entry : initializedSettings->processFactories) {
			//fetchRequest.batchIds.push_back("process-exec");
			if(!entry.second.get().isBusy(*this)) {
				fetchRequest.batchIds.push_back(entry.first);
			}
		}
	}

	/* construct my ids */
	for(const auto entry : settings.ids) {
		service::schemas::Setting setting;

		setting.key = entry.first;
		setting.value = entry.second;

		fetchRequest.ids.push_back(setting);
	}

	/* construct metrics */
	{
		auto metrics = createMetrics(processesRunningCurrent);

		for(const auto& metric : metrics) {
			service::schemas::Setting setting;

			setting.key = metric.first;
			setting.value = metric.second;

			fetchRequest.metrics.push_back(setting);
		}
	}

	std::vector<std::string> processEntriesToDelete;

	/* Construct current jobs status */
	for(const auto& entry : processByJobId) {
		if(!entry.second) {
			logger.warn << "Cannot send status of empty job for job id \"" << entry.first << " \"\n";
			continue;
		}

		auto state = entry.second->getState();

		if(state != common::types::State::Type::running) {
			processEntriesToDelete.push_back(entry.first);
		}

		service::schemas::JobStatusWorker job;

		job.jobId = entry.first;
		job.state = common::types::State::toString(state);
		job.returnCode = entry.second->getRC();
		job.message = entry.second->getMessage();

		fetchRequest.jobs.push_back(job);
	}

	// Send request and receive response
	service::schemas::FetchResponse fetchResponse = service.fetchJob(fetchRequest);

	// After communication was successful we store amount of cummunicated running jobs
	//unpublishedStopStatus = processesRunningCurrent > 0;

	// After communication was successful we can remove old processes
	for(const auto& jobId : processEntriesToDelete) {
		processByJobId.erase(jobId);
	}

	for(const auto& signal : fetchResponse.signals) {
		auto iter = processByJobId.find(signal.jobId);
		if(iter == processByJobId.end()) {
			logger.warn << "Cannot send signal " << signal.signal << " to unknown job id \"" << signal.jobId << "\"\n";
		}
		else if(!iter->second) {
			logger.warn << "Cannot send signal " << signal.signal << " to an empty job id \"" << signal.jobId << "\"\n";
		}
		else {
			esl::utility::Signal s(signal.signal);
			iter->second->sendSignal(s);
		}
	}

	if(!stopping) {
		for(const auto& runConfiguration : fetchResponse.runConfigurations) {
			logger.info << "Run new job.\n";
			logger.info << "- JobId : \"" << runConfiguration.jobId << "\"\n";
			logger.info << "- BatchId : \"" << runConfiguration.batchId << "\"\n";
			auto iter = initializedSettings->processFactories.find(runConfiguration.batchId);
			if(iter == initializedSettings->processFactories.end()) {
				logger.warn << "Cannot create a job for an unknown batch id \"" << runConfiguration.batchId << "\"\n";
			}
			else {
				std::vector<std::pair<std::string, std::string>> settings;
				for(const auto& setting : runConfiguration.settings) {
					settings.emplace_back(setting.key, setting.value);
				}

				processByJobId[runConfiguration.jobId] = iter->second.get().createProcess(*this, settings);
				//unpublishedStopStatus = true;
			}
		}
	}
	else if(!fetchResponse.runConfigurations.empty()) {
		logger.warn << "Received " << fetchResponse.runConfigurations.size() << " jobs to run, but eventStop has been received.\n";
	}

	logger.debug << fetchResponse.runConfigurations.size() << " jobs fetched.\n";
}

std::mutex& Procedure::getMutex() {
	return eventMutex;
}

std::size_t Procedure::calculateProcessesRunning() const {
	std::size_t processesRunning = 0;

	for(const auto& entry : processByJobId) {
		if(entry.second && entry.second->getState() == common::types::State::Type::running) {
			++processesRunning;
		}
	}

	return processesRunning;
}

void Procedure::sendSignalToProcessesRunning(esl::utility::Signal signal) const {
	for(const auto& entry : processByJobId) {
		if(!entry.second) {
			logger.warn << "Cannot send status to job id \"" << entry.first << " \" because it is empty\n";
			continue;
		}

		if(entry.second->getState() != common::types::State::Type::running) {
			continue;
		}

		entry.second->sendSignal(signal);
	}
}

std::vector<std::pair<std::string, std::string>> Procedure::createMetrics(std::size_t processesRunning) {
	std::vector<std::pair<std::string, std::string>> metrics;

	metrics.emplace_back("cpu", "0");
	metrics.emplace_back("processes-running", std::to_string(processesRunning));

	return metrics;
}

std::unique_ptr<esl::com::http::client::Connection> Procedure::createHttpClientConnection() {
	std::unique_ptr<esl::com::http::client::Connection> connection = initializedSettings->connectionFactories[currentConnectionFactoryIndex].get().createConnection();
	if(!connection) {
		throw esl::system::Stacktrace::add(std::runtime_error("Could not create connection"));
	}

	rotateHttpClientConnection();

	return connection;
}

void Procedure::rotateHttpClientConnection() {
	currentConnectionFactoryIndex = (currentConnectionFactoryIndex + 1) % initializedSettings->connectionFactories.size();
}

} /* namespace worker */
} /* namespace batchelor */
