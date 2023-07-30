#ifndef BATCHELOR_CONTROL_OPTIONS_H_
#define BATCHELOR_CONTROL_OPTIONS_H_

#include <batchelor/control/Command.h>
#include <batchelor/control/State.h>
#include <batchelor/control/Server.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace batchelor {
namespace control {

class Options {
public:
	Options(int argc, const char* argv[]);

	void setCommand(const std::string& commandStr);
	const Command& getCommand() const;

	void setEventType(const char* eventType);
	const std::string& getEventType() const noexcept;

	void setPriority(const char* priority);
	int getPriority() const noexcept;

	void addSetting(const char* key, const char* value);
	const std::vector<std::pair<std::string, std::string>>& getSettings() const noexcept;

	void setCondition(const char* condition);
	const std::string& getCondition() const noexcept;

	void setWait();
	bool getWait() const noexcept;

	void setTaskId(const char* taskId);
	const std::string& getTaskId() const noexcept;

	void setSignalName(const char* signalName);
	const std::string& getSignalName() const noexcept;

	void setSignalNumber(const char* signalNumber);
	int getSignalNumber() const noexcept;

	void setState(const char* state);
	State getState() const;

	void setEventNotAfter(const char* eventNotAfter);
	const std::string& getEventNotAfter() const noexcept;

	void setEventNotBefore(const char* eventNotBefore);
	const std::string& getEventNotBefore() const noexcept;

	void addConnectionFile(const char* value);
	void setUsername(const char* value);
	void setPassword(const char* value);
	void addURL(const char* value);
	const std::vector<Server>& getServers() const noexcept;

	static void printUsage();

private:
	std::unique_ptr<Command> command;
	std::string eventType;
	int priority = -1;
	std::vector<std::pair<std::string, std::string>> settings;
	std::string condition;
	bool wait = false;
	std::string taskId;
	std::string signalName;
	int signalNumber = -1;
	std::unique_ptr<State> state;
	std::string eventNotAfter;
	std::string eventNotBefore;

	// connection options
	std::string currentUsername;
	std::string currentPassword;
	std::vector<Server> servers;
};
/*
std::cout << "  batchelor send-event   [CONNECTION OPTIONS] --event-type <event-type> [--priority <priority>] [--setting <key> <value>] [--condition <condition>] [--wait]\n";
std::cout << "  batchelor wait-task    [CONNECTION OPTIONS] --task-id <task-id>\n";
std::cout << "  batchelor cancel-task  [CONNECTION OPTIONS] --task-id <task-id>\n";
std::cout << "  batchelor signal-task  [CONNECTION OPTIONS] --task-id <task-id> [--signal-name <signal-name> | --signal-no <signal number>]\n";
std::cout << "  batchelor show-task    [CONNECTION OPTIONS] --task-id <task-id>\n";
std::cout << "  batchelor show-tasks   [CONNECTION OPTIONS] [--state <state>] [--event-not-after <timestamp>] [--event-not-before <timestamp>]\n";
*/
} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_OPTIONS_H_ */