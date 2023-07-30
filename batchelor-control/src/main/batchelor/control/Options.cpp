#include <batchelor/control/ArgumentsException.h>
#include <batchelor/control/Options.h>

#include <cstring>
#include <iostream>

namespace batchelor {
namespace control {
namespace {

static const std::string commandStrHelp = "help";
static const std::string commandStrSendEvent = "send-event";
static const std::string commandStrWaitTask = "wait-task";
static const std::string commandStrSignalTask = "signal-task";
static const std::string commandStrShowTask = "show-task";
static const std::string commandStrShowTasks = "show-tasks";

const std::string& commandToStr(Command command) noexcept {
	static const std::string commandStrUnknown = "(unknown)";

	switch(command) {
	case Command::sendEvent:
		return commandStrSendEvent;
	case Command::waitTask:
		return commandStrWaitTask;
	case Command::signalTask:
		return commandStrSignalTask;
	case Command::showTask:
		return commandStrShowTask;
	case Command::showTasks:
		return commandStrShowTasks;
	case Command::help:
	//default:
		return commandStrHelp;
	}
	return commandStrUnknown;
}

Command strToCommand(const std::string& commandStr) {
	if(commandStr == commandStrHelp) {
		return Command::help;
	}
	if(commandStr == commandStrSendEvent) {
		return Command::sendEvent;
	}
	if(commandStr == commandStrWaitTask) {
		return Command::waitTask;
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
	throw ArgumentsException("'" + commandStr + "' is no valid command.");
}

static const std::string stateStrWaiting = "waiting";
static const std::string stateStrTimeout = "timeout";
static const std::string stateStrRunning = "running";
static const std::string stateStrDone = "done";
static const std::string stateStrFailed = "failed";
static const std::string stateStrZombi = "zombi";

State strToState(const std::string& stateStr) {
	if(stateStr == stateStrWaiting) {
		return State::waiting;
	}
	if(stateStr == stateStrTimeout) {
		return State::timeout;
	}
	if(stateStr == stateStrRunning) {
		return State::running;
	}
	if(stateStr == stateStrDone) {
		return State::done;
	}
	if(stateStr == stateStrFailed) {
		return State::failed;
	}
	if(stateStr == stateStrZombi) {
		return State::zombi;
	}
	throw ArgumentsException("Invalid value '" + stateStr + "' of option \"--state\".");
}
}

Options::Options(int argc, const char* argv[]) {
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(argv[i][0] != '-') {
			setCommand(currentArg);
		}
		else if(currentArg == "-e"  || currentArg == "--event-type") {
			setEventType(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-p"  || currentArg == "--priority") {
			setPriority(i+1 < argc ? argv[i+1] : nullptr);
			++i;
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
		else if(currentArg == "-t"  || currentArg == "--task-id") {
			setTaskId(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-C"  || currentArg == "--signal") {
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
		else if(currentArg == "-U"  || currentArg == "--server-url") {
			addURL(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-u"  || currentArg == "--username") {
			setUsername(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else if(currentArg == "-p"  || currentArg == "--password") {
			setPassword(i+1 < argc ? argv[i+1] : nullptr);
			++i;
		}
		else {
			throw ArgumentsException("Unknown option '" + currentArg + "'.");
		}
	}

	if(getCommand() != Command::help) {
		if(servers.empty()) {
			throw ArgumentsException("Servers are missing.");
		}
	}

	if(getCommand() == Command::sendEvent) {
		if(getEventType().empty()) {
			throw ArgumentsException("Option \"--event-type\" is missing.");
		}
	}

	switch(getCommand()) {
	case Command::waitTask:
	case Command::signalTask:
	case Command::showTask:
		if(getTaskId().empty()) {
			throw ArgumentsException("Option '--task-id' is missing.");
		}
		break;
	default:
		break;
	}

	if(getCommand() == Command::signalTask) {
		if(getSignal().empty()) {
			throw ArgumentsException("Option \"--signal\" is missing.");
		}
	}
}

void Options::setCommand(const std::string& commandStr) {
	if(command) {
		throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed after previous specification of command \"" + commandToStr(*command) + "\".");
	}

	command.reset(new Command(strToCommand(commandStr)));
	switch(*command) {
	case Command::help:
		break;
	case Command::sendEvent:
		if(!getTaskId().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--task-id'.");
		}
		if(!getSignal().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--signal'.");
		}
		if(state) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--state'.");
		}
		if(!getEventNotAfter().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-not-after'.");
		}
		if(!getEventNotBefore().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-not-before'.");
		}
		break;
	case Command::waitTask:
	case Command::showTask:
		if(!getSignal().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--signal'.");
		}
		// @suppress("No break at end of case")
	case Command::signalTask:
		if(!getEventType().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-type'.");
		}
		if(getPriority() >= 0) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--priority'.");
		}
		if(!getSettings().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--setting'.");
		}
		if(!getCondition().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--condition'.");
		}
		if(getWait()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--wait'.");
		}
		if(state) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--state'.");
		}
		if(!getEventNotAfter().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-not-after'.");
		}
		if(!getEventNotBefore().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-not-before'.");
		}
		break;
	case Command::showTasks:
		if(!getTaskId().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--task-id'.");
		}
		if(!getSignal().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--signal'.");
		}
		if(!getEventType().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--event-type'.");
		}
		if(getPriority() >= 0) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--priority'.");
		}
		if(!getSettings().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--setting'.");
		}
		if(!getCondition().empty()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--condition'.");
		}
		if(getWait()) {
			throw ArgumentsException("Specification of command \"" + commandStr + "\" is not allowed together with option '--wait'.");
		}
		break;
	}
}

const Command& Options::getCommand() const {
	if(!command) {
		throw ArgumentsException("No command specified.");
	}
	return *command;
}

void Options::setEventType(const char* aEventType) {
	if(!eventType.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-type\" is not allowed.");
	}
	if(!aEventType) {
		throw ArgumentsException("Value missing of option \"--event-type\".");
	}

	eventType = aEventType;

	if(!command && *command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--event-type\".");
	}
}

const std::string& Options::getEventType() const noexcept {
	return eventType;
}

void Options::setPriority(const char* aPriority) {
	if(priority >= 0) {
		throw ArgumentsException("Multiple specification of option \"--priority\" is not allowed.");
	}
	if(!aPriority) {
		throw ArgumentsException("Value missing of option \"--priority\".");
	}

	try {
		priority = std::stoi(aPriority);
	}
	catch(const std::invalid_argument& e) {
		throw ArgumentsException("Invalid value '" + std::string(aPriority) + "' of option \"--priority\".");
	}
	catch(const std::out_of_range& e) {
		throw ArgumentsException("Value '" + std::string(aPriority) + "' of option \"--priority\" is out of range. The value should be between 0 and 99.");
	}
	if(priority < 0 || priority > 99) {
		throw ArgumentsException("Value '" + std::string(aPriority) + "' of option \"--priority\" is out of range. The value should be between 0 and 99.");
	}

	if(!command && *command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--priority\".");
	}
}

int Options::getPriority() const noexcept {
	return priority < 0 ? 0 : priority;
}

void Options::addSetting(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--setting\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--setting\".");
	}
	settings.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
}

const std::vector<std::pair<std::string, std::string>>& Options::getSettings() const noexcept {
	return settings;
}

void Options::setCondition(const char* aCondition) {
	if(!condition.empty()) {
		throw ArgumentsException("Multiple specification of option \"--condition\" is not allowed.");
	}
	if(!aCondition) {
		throw ArgumentsException("Value missing of option \"--condition\".");
	}

	condition = aCondition;

	if(!command && *command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--condition\".");
	}
}

const std::string& Options::getCondition() const noexcept {
	static const std::string defaultCondition = "${TRUE}";
	return condition.empty() ? defaultCondition : condition;
}

void Options::setWait() {
	if(wait) {
		throw ArgumentsException("Multiple specification of option \"--wait\" is not allowed.");
	}

	wait = true;

	if(!command && *command != Command::sendEvent) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--wait\".");
	}
}

bool Options::getWait() const noexcept {
	return wait;
}

void Options::setTaskId(const char* value) {
	if(!taskId.empty()) {
		throw ArgumentsException("Multiple specification of option \"--task-id\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--task-id\".");
	}

	taskId = value;

	if(!command && *command != Command::waitTask && *command != Command::signalTask && *command != Command::showTask) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--task-id\".");
	}
}

const std::string& Options::getTaskId() const noexcept {
	return taskId;
}

void Options::setSignal(const char* value) {
	if(!signal.empty()) {
		throw ArgumentsException("Multiple specification of option \"--signal\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--signal\".");
	}

	signal = value;

	if(!command && *command != Command::signalTask) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--signal-name\".");
	}
}

const std::string& Options::getSignal() const noexcept {
	return signal;
}

void Options::setState(const char* value) {
	if(state) {
		throw ArgumentsException("Multiple specification of option \"--state\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--state\".");
	}

	state.reset(new State(strToState(value)));

	if(!command && *command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--state\".");
	}
}

State Options::getState() const {
	if(!state) {
		throw ArgumentsException("Option '--state' is missing.");
	}
	return *state;
}

void Options::setEventNotAfter(const char* value) {
	if(!eventNotAfter.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-not-after\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--event-not-after\".");
	}

	eventNotAfter = value;

	if(!command && *command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--event-not-after\".");
	}
}

const std::string& Options::getEventNotAfter() const noexcept {
	return eventNotAfter;
}

void Options::setEventNotBefore(const char* value) {
	if(!eventNotBefore.empty()) {
		throw ArgumentsException("Multiple specification of option \"--event-not-before\" is not allowed.");
	}
	if(!value) {
		throw ArgumentsException("Value missing of option \"--event-not-before\".");
	}

	eventNotBefore = value;

	if(!command && *command != Command::showTasks) {
		throw ArgumentsException("Command \"" + commandToStr(*command) + "\" does not allow to use option \"--event-not-before\".");
	}
}

const std::string& Options::getEventNotBefore() const noexcept {
	return eventNotBefore;
}

void Options::addConnectionFile(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--connection-file\".");
	}
}

void Options::setUsername(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--username\".");
	}
	currentUsername = value;
}

void Options::setPassword(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--password\".");
	}
	currentPassword = value;
}

void Options::addURL(const char* value) {
	if(!value) {
		throw ArgumentsException("Value missing of option \"--server-url\".");
	}

	Server server;
	server.username = currentUsername;
	server.password = currentPassword;
	server.url = value;

	servers.push_back(server);
}

const std::vector<Server>& Options::getServers() const noexcept {
	return servers;
}

void Options::printUsage() {
	std::cout << "batchelor COMMAND [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor help\n";
	std::cout << "  batchelor send-event   [CONNECTION OPTIONS] --event-type <event-type> [--priority <priority>] [--setting <key> <value>] [--condition <condition>] [--wait]\n";
	std::cout << "  batchelor wait-task    [CONNECTION OPTIONS] --task-id <task-id>\n";
	std::cout << "  batchelor cancel-task  [CONNECTION OPTIONS] --task-id <task-id>\n";
	std::cout << "  batchelor signal-task  [CONNECTION OPTIONS] --task-id <task-id> [--signal-name <signal-name> | --signal-no <signal number>]\n";
	std::cout << "  batchelor show-task    [CONNECTION OPTIONS] --task-id <task-id>\n";
	std::cout << "  batchelor show-tasks   [CONNECTION OPTIONS] [--state <state>] [--event-not-after <timestamp>] [--event-not-before <timestamp>]\n";
	std::cout << "\n";
	std::cout << "COMMANDS:\n";
	std::cout << "  help         shows this help\n";
	std::cout << "  send-event   adds a new event that will wait to get processed.\n";
	std::cout << "  wait-task    Wait for new messages of the given task and return with exit code of this task.\n";
	std::cout << "  cancel-task  Send cancel message to the given task.\n";
	std::cout << "  signal-task  Send a signal to the given task. It must be exactly one signal specified as name or number.\n";
	std::cout << "  show-task    Shows all details of the given task.\n";
	std::cout << "  show-tasks   Shows a list with some attributes of tasks that matches the given criteria.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'send-event':\n";
	std::cout << "  -e, --event-type       <event-type>     Tells the severs which kind of event has to be processed.\n";
	std::cout << "                                          There must be at least one active worker that is processing this event.\n";
	std::cout << "  -p, --priority         <priority>       Tells the head to process this event with a specific priority. Default value is 0.\n";
	std::cout << "  -s, --setting          <key> <value>    This option is allowed to be used multible times. The settings are specific to the event type.\n";
	std::cout << "  -c, --condition        <condition>      Formula that specifies if a worker is allowed to process this event.\n";
	std::cout << "  -w, --wait                              Wait for new messages and return with exit code of task.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'wait-task', 'cancel-task', 'signal-task', 'show-task':\n";
	std::cout << "  -t, --task-id          <task-id>        Specifies the task that the command is related to.\n";
	std::cout << "\n";
	std::cout << "OPTIONS specific for command 'signal-task':\n";
	std::cout << "  -C, --signal <signal>                   Specifies the signal that will be sent to the task. Signal \"CANCEL\" has a special meaning\n";
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
	std::cout << "                                          Available states are 'waiting', 'timeout', 'running', 'done', 'failed' and 'zombi'.\n";
	std::cout << "  -A, --event-not-after  <timestamp>      Specifies a filter to select events that has been sent not after the specified timestamp.\n";
	std::cout << "  -B, --event-not-before <timestamp>      Specifies a filter to select events that has been sent not before the specified timestamp.\n";
	std::cout << "                                          Format of <timestamp> is 'YYYY-MM-DD hh:mm:ss'\n";
	std::cout << "\n";
	std::cout << "General CONNECTION OPTIONS:\n";
	std::cout << "  -f, --connection-file  <file>           Connection file can contain all of the following connection options, but addition options are still allowed\n";
	std::cout << "  -U, --server-url       <server-url>     At least one server-url must be specified\n";
	std::cout << "  -u, --username         <username>       If <username> is specified, basic-auth is used\n";
	std::cout << "  -p, --password         <password>       If <password> is specified, basic-auth is used\n";
}

} /* namespace control */
} /* namespace batchelor */
