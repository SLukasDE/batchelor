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

#include <batchelor/common/types/State.h>

#include <batchelor/control/Logger.h>
#include <batchelor/control/Procedure.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/com/http/client/exception/NetworkError.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace control {
namespace {
Logger logger("batchelor::control::Procedure");

void printStatus(const service::schemas::TaskStatusHead& taskStatus) {
	common::types::State::Type state = common::types::State::toState(taskStatus.state);

	logger.info << "State      : \"" << taskStatus.state << "\"\n";
	logger.info << "Created TS : \"" << taskStatus.tsCreated << "\"\n";
	switch(state) {
	case common::types::State::queued:
		break;
	case common::types::State::running:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		if(!taskStatus.message.empty()) {
			logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		}
		break;
	case common::types::State::done:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskStatus.tsFinished << "\"\n";
		logger.info << "Return code: \"" << taskStatus.returnCode << "\"\n";
		if(!taskStatus.message.empty()) {
			logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		}
		break;
	case common::types::State::signaled:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskStatus.tsFinished << "\"\n";
		logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		break;
	case common::types::State::zombie:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Message    : \"" << taskStatus.message << "\"\n";
		break;
	}
}
}

Procedure::InitializedSettings::InitializedSettings(esl::object::Context& context, const Settings& settings) {
	for(const auto& connectionId : settings.connectionFactoryIds) {
		common::plugin::ConnectionFactory& connectionFactory = context.getObject<common::plugin::ConnectionFactory>(connectionId);
		connectionFactories.emplace_back(connectionId, std::ref(connectionFactory));
	}
}

Procedure::Procedure(const Settings& aSettings)
: settings(aSettings)
{
	if(settings.connectionFactoryIds.empty()) {
		throw std::runtime_error("No connections defined");
	}
}

void Procedure::procedureCancel() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		++signalsReceived;
	}
	notifyCV.notify_one();
}

void Procedure::initializeContext(esl::object::Context& context) {
	initializedSettings.reset(new InitializedSettings(context, settings));
}

int Procedure::getReturnCode() const noexcept {
	return rc;
}

void Procedure::internalProcedureRun(esl::object::Context& context) {
	if(!settings.command) {
		throw std::runtime_error("No command specified.");
	}

	std::size_t firstConnectionFactory = nextConnectionFactory;
	do {
		try {
			switch(*settings.command) {
			case Command::sendEvent:
				sendEvent();
				break;
			case Command::waitTask:
				waitTask(settings.taskId);
				break;
			case Command::cancelTask:
				signalTask(settings.taskId, "CANCEL");
				break;
			case Command::signalTask:
				signalTask(settings.taskId, settings.signal);
				break;
			case Command::showTask:
				showTask();
				break;
			case Command::showTasks:
				showTasks();
				break;
			case Command::showEventTypes:
				showEventTypes();
				break;
			}
			break;
		}
		catch(const esl::com::http::client::exception::NetworkError& e) {
			logger.warn << "Network error: " << e.what() << "\n";
			httpConnectionFactory = nullptr;
		}
	} while(firstConnectionFactory != nextConnectionFactory);
}

void Procedure::sendEvent() {
	service::schemas::RunResponse runResponse;
	{
		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		service::schemas::RunRequest runRequest;
		runRequest.eventType = settings.eventType;
		runRequest.priority = settings.priority < 0 ? 0 : settings.priority;
		for(const auto& setting : settings.settings) {
			runRequest.settings.push_back(service::schemas::Setting::make(setting.first, setting.second));
		}
		for(const auto& metric : settings.metrics) {
			runRequest.metrics.push_back(service::schemas::Setting::make(metric.first, metric.second));
		}
		runRequest.condition = settings.condition;

		runResponse = client.runTask(settings.namespaceId, runRequest);

		if(runResponse.taskId.empty()) {
			rc = 1;
			logger.info << "Message    : \"" << runResponse.message << "\"\n";
			logger.info << "-----------------\n";
			return;
		}
		logger.info << "Task ID    : \"" << runResponse.taskId << "\"\n";
	}

	if(settings.wait || settings.waitCancel != -2) {
		logger.info << "-----------------\n";
		waitTask(runResponse.taskId);
	}
}

void Procedure::waitTask(const std::string& taskId) {
	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
	service::schemas::TaskStatusHead oldTaskStatus;

	{
		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(settings.namespaceId, taskId);
		if(!taskStatus) {
			return;
		}

		rc = taskStatus->returnCode;

		printStatus(*taskStatus);

		oldTaskStatus = *taskStatus;
	}

	while(true) {
		{
			common::types::State::Type state = common::types::State::toState(oldTaskStatus.state);
			if(state == common::types::State::done || state == common::types::State::signaled || state == common::types::State::zombie) {
				break;
			}
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		notifyCV.wait_for(lockNotifyMutex, std::chrono::milliseconds(5000));
		if(settings.waitCancel != -2 && signalsReceived > signalsProcessed) {
			++signalsProcessed;
			signalTask(taskId, "CANCEL");
		}
		if((settings.waitCancel == -2 && signalsReceived > 0)
		|| (settings.waitCancel >=  0 && signalsReceived > settings.waitCancel)) {
			break;
		}

		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(settings.namespaceId, taskId);
		if(!taskStatus) {
			return;
		}

		if(oldTaskStatus.state == taskStatus->state && oldTaskStatus.message == taskStatus->message) {
			continue;
		}

		rc = taskStatus->returnCode;

		logger.info << "-----------------\n";
		printStatus(*taskStatus);

		oldTaskStatus = *taskStatus;
	}
}

void Procedure::signalTask(const std::string& taskId, const std::string& signal) {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	client.sendSignal(settings.namespaceId, taskId, signal);
}

void Procedure::showTask() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::unique_ptr<service::schemas::TaskStatusHead> taskHead = client.getTask(settings.namespaceId, settings.taskId);
	if(taskHead) {
		showTask(*taskHead);
	}
}

void Procedure::showTasks() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::vector<service::schemas::TaskStatusHead> taskHeads = client.getTasks(settings.namespaceId, settings.state, settings.eventNotAfter, settings.eventNotBefore);
	if(taskHeads.size() == 1) {
		logger.info << "1 entry:\n";
	}
	else {
		logger.info << taskHeads.size() << " entries:\n";
	}
	for(std::size_t i = 0; i < taskHeads.size(); ++i) {
		logger.info << "-----------------\n";
		logger.info << "#" << (i+1) << ":\n";
		showTask(taskHeads[i]);
	}
}

void Procedure::showEventTypes() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::vector<std::string> eventTypes = client.getEventTypes(settings.namespaceId);
	if(eventTypes.size() == 1) {
		logger.info << "1 entry:\n";
	}
	else {
		logger.info << eventTypes.size() << " entries:\n";
	}
	for(std::size_t i = 0; i < eventTypes.size(); ++i) {
		logger.info << "#" << (i+1) << ": \"" << eventTypes[i] << "\"\n";
	}
}


std::unique_ptr<esl::com::http::client::Connection> Procedure::createHTTPConnection() const {
	if(!httpConnectionFactory && initializedSettings && nextConnectionFactory < initializedSettings->connectionFactories.size()) {
		httpConnectionFactory = &initializedSettings->connectionFactories[nextConnectionFactory].second.get();
		nextConnectionFactory = (nextConnectionFactory + 1) % initializedSettings->connectionFactories.size();
	}
	if(!httpConnectionFactory) {
		if(!initializedSettings) {
			logger.warn << "InizializeContext has not been called.\n";
		}
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}

	auto httpConnection = httpConnectionFactory->get().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

void Procedure::showTask(const service::schemas::TaskStatusHead& taskHead) const noexcept {
	logger.info << "Task ID    : \"" << taskHead.runConfiguration.taskId << "\"\n";
	logger.info << "Event type : \"" << taskHead.runConfiguration.eventType << "\"\n";
	if(taskHead.runConfiguration.settings.empty()) {
		logger.info << "Settings   : (empty)\n";
	}
	else {
		logger.info << "Settings   :\n";
		for(std::size_t i = 0; i<taskHead.runConfiguration.settings.size(); ++i) {
			logger.info << "  [" << (i+1) << "]: \"" << taskHead.runConfiguration.settings[i].key << "\" = \"" << taskHead.runConfiguration.settings[i].value << "\"\n";
		}

	}
	logger.info << "Metrics    :\n";
	for(std::size_t i = 0; i<taskHead.runConfiguration.metrics.size(); ++i) {
		logger.info << "  [" << (i+1) << "]: \"" << taskHead.runConfiguration.metrics[i].key << "\" = \"" << taskHead.runConfiguration.metrics[i].value << "\"\n";
	}
	logger.info << "State      : \"" << taskHead.state << "\"\n";
	logger.info << "Return code: \"" << taskHead.returnCode << "\"\n";
	logger.info << "Message    : \"" << taskHead.message << "\"\n";
	logger.info << "Created TS : \"" << taskHead.tsCreated << "\"\n";
	logger.info << "Running TS : \"" << taskHead.tsRunning << "\"\n";
	logger.info << "Finished TS: \"" << taskHead.tsFinished << "\"\n";
	logger.info << "Heartbeat  : \"" << taskHead.tsLastHeartBeat << "\"\n";
}

} /* namespace control */
} /* namespace batchelor */
