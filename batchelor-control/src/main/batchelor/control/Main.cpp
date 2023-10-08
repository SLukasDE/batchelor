/*
 * This file is part of Batchelor.
 * Copyright (C) 2023 Sven Lukas
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

#include <batchelor/control/Main.h>
#include <batchelor/control/Logger.h>

#include <batchelor/common/types/State.h>

#include <batchelor/service/client/Service.h>
#include <batchelor/service/schemas/RunRequest.h>
#include <batchelor/service/schemas/RunResponse.h>

#include <zsystem4esl/system/signal/Signal.h>

#include <esl/system/Stacktrace.h>

#include <stdexcept>

namespace batchelor {
namespace control {
namespace {
Logger logger("batchelor::control::Main");

void printStatus(const service::schemas::TaskStatusHead& taskStatus) {
	common::types::State::Type state = common::types::State::toState(taskStatus.state);

	logger.info << "State      : \"" << taskStatus.state << "\"\n";
	logger.info << "Created TS : \"" << taskStatus.tsCreated << "\"\n";
	switch(state) {
	case common::types::State::queued:
		break;
	case common::types::State::running:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		break;
	case common::types::State::done:
		logger.info << "Running TS : \"" << taskStatus.tsRunning << "\"\n";
		logger.info << "Finished TS: \"" << taskStatus.tsFinished << "\"\n";
		logger.info << "Return code: \"" << taskStatus.returnCode << "\"\n";
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

Main::Main(const Settings& aSettings)
: settings(aSettings),
  url("http://localhost:8080"),
  signal(new zsystem4esl::system::signal::Signal({}))
{
	std::set<std::string> stopSignals{ {"interrupt"}, {"terminate"}, {"pipe"} };
	for(auto stopSignal : stopSignals) {
		signalHandles.push_back(signal->createHandler(esl::utility::Signal(stopSignal), [this]() {
			stopRunning();
		}));
	}

	if(!settings.command) {
		throw std::runtime_error("No command specified.");
	}
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
	}
}

void Main::stopRunning() {
	{
		std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
		++signalsReceived;
	}
	notifyCV.notify_one();
}

void Main::sendEvent() {
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
		runRequest.condition = settings.condition.empty() ? "${TRUE}" : settings.condition;

		runResponse = client.runTask(runRequest);
		logger.info << "Task ID    : \"" << runResponse.taskId << "\"\n";
	}

	if(settings.wait || settings.waitCancel != -2) {
		logger.info << "-----------------\n";
		waitTask(runResponse.taskId);
	}
}

void Main::waitTask(const std::string& taskId) {
	std::unique_lock<std::mutex> lockNotifyMutex(notifyMutex);
	service::schemas::TaskStatusHead oldTaskStatus;

	{
		auto httpConnection = createHTTPConnection();
		service::client::Service client(*httpConnection);

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(taskId);
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

		std::unique_ptr<service::schemas::TaskStatusHead> taskStatus = client.getTask(taskId);
		if(!taskStatus) {
			return;
		}

		if(oldTaskStatus.state == taskStatus->state) {
			continue;
		}

		rc = taskStatus->returnCode;

		logger.info << "-----------------\n";
		printStatus(*taskStatus);

		oldTaskStatus = *taskStatus;
	}
}

void Main::signalTask(const std::string& taskId, const std::string& signal) {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	client.sendSignal(taskId, signal);
}

void Main::showTask() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::unique_ptr<service::schemas::TaskStatusHead> taskHead = client.getTask(settings.taskId);
	if(taskHead) {
		showTask(*taskHead);
	}
}

void Main::showTasks() {
	auto httpConnection = createHTTPConnection();
	service::client::Service client(*httpConnection);

	std::vector<service::schemas::TaskStatusHead> taskHeads = client.getTasks(settings.state, settings.eventNotAfter, settings.eventNotBefore);
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

int Main::getReturnCode() const {
	return rc;
}

std::unique_ptr<esl::com::http::client::Connection> Main::createHTTPConnection() const {
	auto httpConnection = getHTTPConnectionFactory().createConnection();
	if(!httpConnection) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection."));
	}
	return httpConnection;
}

esl::com::http::client::ConnectionFactory& Main::getHTTPConnectionFactory() const {
	if(!httpConnectionFactory) {
		httpConnectionFactory = esl::plugin::Registry::get().create<esl::com::http::client::ConnectionFactory>(
				"eslx/com/http/client/ConnectionFactory", {
						{"url", url}
				});
	}
	if(!httpConnectionFactory) {
		throw esl::system::Stacktrace::add(std::runtime_error("cannot create http connection factory."));
	}
	return *httpConnectionFactory;
}

void Main::showTask(const service::schemas::TaskStatusHead& taskHead) const noexcept {
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
	for(std::size_t i = 0; i<taskHead.metrics.size(); ++i) {
		logger.info << "  [" << (i+1) << "]: \"" << taskHead.metrics[i].key << "\" = \"" << taskHead.metrics[i].value << "\"\n";
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
