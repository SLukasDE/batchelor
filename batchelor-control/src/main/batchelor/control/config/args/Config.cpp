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

#include <batchelor/common/config/args/ArgumentsException.h>
#include <batchelor/common/plugin/ConnectionFactory.h>
#include <batchelor/common/types/State.h>

#include <batchelor/control/config/args/Config.h>

#include <esl/plugin/Registry.h>

#include <iostream>

namespace batchelor {
namespace control {
namespace config {
namespace args {

using batchelor::common::config::args::ArgumentsException;

namespace {

static const std::string commandStrHelp = "help";
static const std::string commandStrSendEvent = "send-event";
static const std::string commandStrWaitTask = "wait-task";
static const std::string commandStrCancelTask = "cancel-task";
static const std::string commandStrSignalTask = "signal-task";
static const std::string commandStrShowTask = "show-task";
static const std::string commandStrShowTasks = "show-tasks";
static const std::string commandStrShowEventTypes = "show-event-types";

const std::string& commandToStr(Command command) noexcept {
	static const std::string commandStrUnknown = "(unknown)";

	switch(command) {
	case Command::sendEvent:
		return commandStrSendEvent;
	case Command::waitTask:
		return commandStrWaitTask;
	case Command::cancelTask:
		return commandStrCancelTask;
	case Command::signalTask:
		return commandStrSignalTask;
	case Command::showTask:
		return commandStrShowTask;
	case Command::showTasks:
		return commandStrShowTasks;
	case Command::showEventTypes:
		return commandStrShowEventTypes;
	}
	return commandStrUnknown;
}

Command strToCommand(const std::string& commandStr) {
	if(commandStr == commandStrHelp) {
		throw ArgumentsException("");
	}
	if(commandStr == commandStrSendEvent) {
		return Command::sendEvent;
	}
	if(commandStr == commandStrWaitTask) {
		return Command::waitTask;
	}
	if(commandStr == commandStrCancelTask) {
		return Command::cancelTask;
	}
	if(commandStr == commandStrSignalTask) {
		return Command::signalTask;
	}
	if(commandStr == commandStrShowTask) {
		return Command::showTask;
	}
	if(commandStr == commandStrShowTasks) {
		return Command::showTasks;
	}
	if(commandStr == commandStrShowEventTypes) {
		return Command::showEventTypes;
	}
	throw ArgumentsException("'" + commandStr + "' is no valid command.");
}

ArgumentsException argumentsExceptionCommandOptionMismatch(const std::string& commandStr, const std::string& optionStr) {
	return ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '" + optionStr + "'.");
}

} /* anonymous namespace */

void Config::printUsage() {
	std::cout << "batchelor-control COMMAND [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor-control help\n";
	std::cout << "  batchelor-control send-event       [CONNECTION OPTIONS] --event-type <event-type> [--priority <priority>] [--setting <key> <value>] [--condition <condition>] [--wait | --wait-cancel <max-tries>]\n";
	std::cout << "  batchelor-control wait-task        [CONNECTION OPTIONS] --task-id <task-id> [--wait-cancel <max-tries>]\n";
	std::cout << "  batchelor-control cancel-task      [CONNECTION OPTIONS] --task-id <task-id>\n";
	std::cout << "  batchelor-control signal-task      [CONNECTION OPTIONS] --task-id <task-id> --signal <signal>\n";
	std::cout << "  batchelor-control show-task        [CONNECTION OPTIONS] --task-id <task-id>\n";
	std::cout << "  batchelor-control show-tasks       [CONNECTION OPTIONS] [--state <state>] [--event-not-after <timestamp>] [--event-not-before <timestamp>]\n";
	std::cout << "  batchelor-control show-event-types [CONNECTION OPTIONS]\n";
	std::cout << "\n";
	std::cout << "COMMANDS:\n";
	std::cout << "  help              shows this help\n";
	std::cout << "  send-event        adds a new event that will wait to get processed.\n";
	std::cout << "  wait-task         Wait for new messages of the given task and return with exit code of this task.\n";
	std::cout << "  cancel-task       This is equal to command 'signal-task' with option '--signal CANCEL'.\n";
	std::cout << "  signal-task       Send a signal to the given task. It must be exactly one signal specified as name or number.\n";
	std::cout << "  show-task         Shows all details of the given task.\n";
	std::cout << "  show-tasks        Shows a list with some attributes of tasks that matches the given criteria.\n";
	std::cout << "  show-event-types  Shows a list of available event types.\n";
	std::cout << "\n";
	std::cout << "General OPTIONS:\n";
	std::cout << "  -N, --namespace        <namespace-id>   Specifies the used namespace for all commands.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'send-event':\n";
	std::cout << "  -m, --metric           <key> <value>    Specifies metrics to be processed by the tasks.\n";
	std::cout << "  -e, --event-type       <event-type>     Tells the severs which kind of event has to be processed.\n";
	std::cout << "                                          There must be at least one active worker that is processing this event.\n";
	std::cout << "  -p, --priority         <priority>       Tells the head to process this event with a specific priority. Default value is 0.\n";
	std::cout << "  -s, --setting          <key> <value>    Event type or connection specific setting.\n";
	std::cout << "  -c, --condition        <condition>      Formula that specifies if a worker is allowed to process this event.\n";
	std::cout << "  -w, --wait                              Wait for new messages and return with exit code of task.\n";
	std::cout << "  -W, --wait-cancel      <max-tries>      Wait for new messages and return with exit code of task. A CANCEL signal will be send if an abort\n";
	std::cout << "                                          signal is received, but control program does not abort until receiving this signal <max-tries> times.\n";
	std::cout << "                                          If <max-tries> is set to -1 control program will never stop until task has been stopped.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'wait-task', 'cancel-task', 'signal-task', 'show-task':\n";
	std::cout << "  -t, --task-id          <task-id>        Specifies the task that the command is related to.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'signal-task':\n";
	std::cout << "  -g, --signal           <signal>         Specifies the signal that will be sent to the task. Signal \"CANCEL\" has a special meaning\n";
	std::cout << "                                          to cancel a task. If task is still waiting, it is removed from to wait for running.\n";
	std::cout << "                                          If it is already running, it will result to send a special signal equal to Strg+C.\n";
	std::cout << "                                          Other available values are numbers between 1-31 and between 34-64 of following constants:\n";
	std::cout << "                                          SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGKILL, SIGUSR1,\n";
	std::cout << "                                          SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM, SIGTERM, SIGSTKFLT, SIGCHLD, SIGCONT, SIGSTOP,\n";
	std::cout << "                                          SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH,\n";
	std::cout << "                                          SIGIO, SIGPWR, SIGSYS, SIGRTMIN, SIGRTMIN+1, SIGRTMIN+2, SIGRTMIN+3, SIGRTMIN+4,\n";
	std::cout << "                                          SIGRTMIN+5, SIGRTMIN+6, SIGRTMIN+7, SIGRTMIN+8, SIGRTMIN+9, SIGRTMIN+10, SIGRTMIN+11\n";
	std::cout << "                                          SIGRTMIN+12, SIGRTMIN+13, SIGRTMIN+14, SIGRTMIN+15, SIGRTMAX-14, SIGRTMAX-13,\n";
	std::cout << "                                          SIGRTMAX-12, SIGRTMAX-11, SIGRTMAX-10, SIGRTMAX-9, SIGRTMAX-8, SIGRTMAX-7 SIGRTMAX-6,\n";
	std::cout << "                                          SIGRTMAX-5, SIGRTMAX-4, SIGRTMAX-3, SIGRTMAX-2, SIGRTMAX-1, SIGRTMAX\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'show-tasks':\n";
	std::cout << "  -S, --state            <state>          Specifies a filter to select events that that matches to the specified state.\n";
	std::cout << "                                          Available states are 'queued', 'running', 'done', 'signaled' and 'zombi'.\n";
	std::cout << "  -A, --event-not-after  <timestamp>      Specifies a filter to select events that has been sent not after the specified timestamp.\n";
	std::cout << "  -B, --event-not-before <timestamp>      Specifies a filter to select events that has been sent not before the specified timestamp.\n";
	std::cout << "                                          Format of <timestamp> is 'YYYY-MM-DD hh:mm:ss'\n";
	std::cout << "\n";
	std::cout << "CONNECTION OPTIONS:\n";
	std::cout << "  -f, --connection-file  <file>           Connection file can contain all of the following connection options\n";
	std::cout << "  -C, --connection       <plugin>         Defines the connection to a head server.\n";
	std::cout << "                                          Subsequent settings specified by \"--setting\" are specific to the plugin.\n";
	std::cout << "\n";
	std::cout << "                                          Most popular used plugin is \"basic\" with following settings:\n";
	std::cout << "                                          * url:           <server-url>  Defines the URL to the head server.\n";
	std::cout << "                                          * api-key:       <api-key>     If this setting is specified, a bearer token will be used as authorization.\n";
	std::cout << "                                          * username:      <username>    If this setting is specified, basic-auth will be used.\n";
	std::cout << "                                          * password:      <password>    If this setting is specified, basic-auth will be used.\n";
	/*
	std::cout << "\n";
	std::cout << "                                          Another popular plugin is \"oidc\" with following settings:\n";
	std::cout << "                                          * url:           <server-url>  Defines the URL to the head server.\n";
	std::cout << "                                          * oidc-url:      <idp-url>     Defines the URL to the OAuth2 server, if client-id is used.\n";
	std::cout << "                                          * client-id:     <client-id>   If this setting is specified, OIDC protocol is used.\n";
	std::cout << "                                          * client-secret: <client-id>   If this setting is specified, OIDC protocol is used.\n";
	*/
}

Config::Config(esl::object::Context& aContext, Procedure::Settings& aSettings, int argc, const char* argv[])
: context(aContext),
  settings(aSettings)
{
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(argv[i][0] != '-') {
			setCommand(currentArg);
		}
		else if(currentArg == "-N"  || currentArg == "--namespace") {
			setNamespaceId(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-e"  || currentArg == "--event-type") {
			setEventType(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-p"  || currentArg == "--priority") {
			setPriority(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-m"  || currentArg == "--metric") {
			addMetric(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-s"  || currentArg == "--setting") {
			addSetting(i+1 < argc ? argv[i+1] : nullptr, i+2 < argc ? argv[i+2] : nullptr);
			i = i+2;
		}
		else if(currentArg == "-c"  || currentArg == "--condition") {
			setCondition(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-w"  || currentArg == "--wait") {
			setWait();
		}
		else if(currentArg == "-W"  || currentArg == "--wait-cancel") {
			setWaitCancel(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-t"  || currentArg == "--task-id") {
			setTaskId(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-g"  || currentArg == "--signal") {
			setSignal(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-S"  || currentArg == "--state") {
			setState(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-A"  || currentArg == "--event-not-after") {
			setEventNotAfter(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-B"  || currentArg == "--event-not-before") {
			setEventNotBefore(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-f"  || currentArg == "--connection-file") {
			addConnectionFile(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-C"  || currentArg == "--connection") {
			addConnection(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}

	if(getCommand() == Command::sendEvent) {
		if(settings.eventType.empty()) {
			throw ArgumentsException("Option \"--event-type\" is missing.");
		}
	}

	switch(getCommand()) {
	case Command::waitTask:
	case Command::cancelTask:
	case Command::signalTask:
	case Command::showTask:
		if(settings.taskId.empty()) {
			throw ArgumentsException("Option '--task-id' is missing.");
		}
		break;
	default:
		break;
	}

	if(getCommand() == Command::signalTask) {
		if(settings.signal.empty()) {
			throw ArgumentsException("Option \"--signal\" is missing.");
		}
	}

	setSettingState(SettingsState::none);
}

void Config::setCommand(const std::string& commandStr) {
	if(settings.command) {
		throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed after previous specification of command \"" + commandToStr(*settings.command) + "\".");
	}

	settings.command.reset(new Command(strToCommand(commandStr)));
	switch(*settings.command) {
	case Command::sendEvent:
		if(!settings.taskId.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--task-id");
		}
		if(!settings.signal.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--signal");
		}
		if(!settings.state.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--state");
		}
		if(!settings.eventNotAfter.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-after");
		}
		if(!settings.eventNotBefore.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-before");
		}
		break;
	case Command::waitTask:
	case Command::showTask:
		if(!settings.signal.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--signal");
		}
		// @suppress("No break at end of case")
	case Command::cancelTask:
	case Command::signalTask:
		if(!settings.eventType.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-type");
		}
		if(settings.priority >= 0) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--priority");
		}
		if(!settings.settings.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--setting");
		}
		if(!settings.condition.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--condition");
		}
		if(settings.wait) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--wait");
		}
		if(!settings.state.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--state");
		}
		if(!settings.eventNotAfter.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-after");
		}
		if(!settings.eventNotBefore.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-before");
		}
		break;
	case Command::showEventTypes:
		if(!settings.state.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--state");
		}
		if(!settings.eventNotAfter.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-after");
		}
		if(!settings.eventNotBefore.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-not-before");
		}
		// @suppress("No break at end of case")
	case Command::showTasks:
		if(!settings.taskId.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--task-id");
		}
		if(!settings.signal.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--signal");
		}
		if(!settings.eventType.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--event-type");
		}
		if(settings.priority >= 0) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--priority");
		}
		if(!settings.settings.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--setting");
		}
		if(!settings.condition.empty()) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "condition");
		}
		if(settings.wait) {
			throw argumentsExceptionCommandOptionMismatch(commandStr, "--wait");
		}
		break;
	}
}

const Command& Config::getCommand() const {
	if(!settings.command) {
		throw ArgumentsException("No command specified.");
	}
	return *settings.command;
}

const std::vector<std::string>& Config::getConfigFiles() const noexcept {
	return configFiles;
}

void Config::setSettingState(SettingsState aSettingState) {
	if(settingState == SettingsState::connection) {
		++connectionCount;
		std::string id = "batchelor-control-connection-" + std::to_string(connectionCount);

		context.addObject(id, esl::plugin::Registry::get().create<common::plugin::ConnectionFactory>(connection.plugin, connection.settings));
		if(settings.connectionFactoryIds.insert(id).second == false) {
			throw ArgumentsException("Multiple specification of server connection with <id> = \"" + id + "\".");
		}
	}

	else if(settingState == SettingsState::event) {
		/*
		context.addObject(event.id, esl::plugin::Registry::get().create<plugin::TaskFactory>(event.type, event.settings));
		if(settings.taskFactoryIds.insert(event.id).second == false) {
			throw ArgumentsException("Multiple specification of option \"--event-type\" with <id> = \"" + event.id + "\".");
		}
		*/
	}

	settingState = aSettingState;

	connection = Connection();
}

void Config::setNamespaceId(const char* namespaceId) {
	if(!settings.namespaceId.empty()) {
		throw ArgumentsException("Multiple specification of option \"--namespace\" is not allowed.");
	}
	if(!namespaceId) {
		throw ArgumentsException("Value missing of option \"--namespace\".");
	}

	setSettingState(SettingsState::event);

	settings.namespaceId = namespaceId;
}

void Config::setEventType(const char* eventType) {
	if(!settings.eventType.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-type\" is not allowed.");
	}
	if(!eventType) {
		throw ArgumentsException("Value missing of option \"--event-type\".");
	}

	setSettingState(SettingsState::event);

	settings.eventType = eventType;

	if(settings.command && *settings.command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--event-type\".");
	}
}

void Config::setPriority(const char* aPriority) {
	if(settings.priority >= 0) {
		throw ArgumentsException("Multiple specification of option \"--priority\" is not allowed.");
	}
	if(!aPriority) {
		throw ArgumentsException("Value missing of option \"--priority\".");
	}

	try {
		settings.priority = std::stoi(aPriority);
	}
	catch(const std::invalid_argument& e) {
		throw ArgumentsException("Invalid value '" + std::string(aPriority) + "' of option \"--priority\".");
	}
	catch(const std::out_of_range& e) {
		throw ArgumentsException("Value '" + std::string(aPriority) + "' of option \"--priority\" is out of range. The value should be between 0 and 99.");
	}
	if(settings.priority < 0 || settings.priority > 99) {
		throw ArgumentsException("Value '" + std::string(aPriority) + "' of option \"--priority\" is out of range. The value should be between 0 and 99.");
	}

	if(settings.command && *settings.command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--priority\".");
	}
}

void Config::addMetric(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--metric\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--metric\".");
	}

	settings.metrics.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
}

void Config::addSetting(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--setting\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--setting\".");
	}

	switch(settingState) {
	case SettingsState::none:
		throw ArgumentsException("Option \"--setting\" is invalid, because there is no previous option \"--connection\" or \"--event-type\".");
	case SettingsState::event:
		settings.settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
		break;
	case SettingsState::connection:
		connection.settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
	}
}

void Config::setCondition(const char* aCondition) {
	if(!settings.condition.empty()) {
		throw ArgumentsException("Multiple specification of option \"--condition\" is not allowed.");
	}
	if(!aCondition) {
		throw ArgumentsException("Value missing of option \"--condition\".");
	}

	settings.condition = aCondition;

	if(settings.command && *settings.command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--condition\".");
	}
}

void Config::setWait() {
	if(settings.wait) {
		throw ArgumentsException("Multiple specification of option \"--wait\" is not allowed.");
	}
	if(settings.waitCancel >= -1) {
		throw ArgumentsException("Specification of option \"--wait\" is not allowed together with option \"--wait-cancel'.");
	}

	settings.wait = true;

	if(settings.command && *settings.command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--wait\".");
	}
}

void Config::setWaitCancel(const char* aMaxTries) {
	if(settings.waitCancel >= -1) {
		throw ArgumentsException("Multiple specification of option \"--wait-cancel\" is not allowed.");
	}
	if(!aMaxTries) {
		throw ArgumentsException("Value missing of option \"--wait-cancel\".");
	}
	if(settings.wait) {
		throw ArgumentsException("Specification of option \"--wait-cancel\" is not allowed together with option \"--wait'.");
	}

	settings.waitCancel = std::stoi(aMaxTries);
	if(settings.waitCancel <= -2) {
		throw ArgumentsException("Value " + std::to_string(settings.waitCancel) + " is not allowed for option \"--wait-cancel\". Value must be -1 or greater.");
	}

	if(settings.command && *settings.command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--wait-cancel\".");
	}
}

void Config::setTaskId(const char* value) {
	if(!settings.taskId.empty()) {
		throw ArgumentsException("Multiple specification of option \"--task-id\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--task-id\".");
	}

	settings.taskId = value;

	if(settings.command && *settings.command != Command::waitTask && *settings.command != Command::cancelTask && *settings.command != Command::signalTask && *settings.command != Command::showTask) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--task-id\".");
	}
}

void Config::setSignal(const char* value) {
	if(!settings.signal.empty()) {
		throw ArgumentsException("Multiple specification of option \"--signal\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--signal\".");
	}

	settings.signal = value;

	if(settings.command && *settings.command != Command::signalTask) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--signal-name\".");
	}
}

void Config::setState(const char* value) {
	if(!settings.state.empty()) {
		throw ArgumentsException("Multiple specification of option \"--state\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--state\".");
	}

	common::types::State::Type(common::types::State::toState(value));
	settings.state = value;

	if(settings.state.empty()) {
		throw ArgumentsException("Definition of invalid value \"\" for option \"--state\".");
	}

	if(settings.command && *settings.command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--state\".");
	}
}

void Config::setEventNotAfter(const char* value) {
	if(!settings.eventNotAfter.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-not-after\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--event-not-after\".");
	}

	settings.eventNotAfter = value;

	if(settings.command && *settings.command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--event-not-after\".");
	}
}

void Config::setEventNotBefore(const char* value) {
	if(!settings.eventNotBefore.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-not-before\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--event-not-before\".");
	}

	settings.eventNotBefore = value;

	if(settings.command && *settings.command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*settings.command) + "\" does not allow to use option \"--event-not-before\".");
	}
}

void Config::addConnection(const char* plugin) {
	if(!plugin) {
		throw ArgumentsException("Plugin-value missing of option \"--connection\".");
	}

	setSettingState(SettingsState::connection);
	connection.plugin = plugin;
}

void Config::addConnectionFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--connection-file\".");
	}
	configFiles.push_back(value);
}

} /* namespace args */
} /* namespace config */
} /* namespace control */
} /* namespace batchelor */
