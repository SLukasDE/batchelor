#ifndef BATCHELOR_CONTROL_OPTIONS_H_
#define BATCHELOR_CONTROL_OPTIONS_H_

#include <batchelor/common/Server.h>
#include <batchelor/common/types/State.h>

#include <batchelor/control/Command.h>

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

	void setSignal(const char* signalName);
	const std::string& getSignal() const noexcept;

	void setState(const char* state);
	common::types::State::Type getState() const;

	void setEventNotAfter(const char* eventNotAfter);
	const std::string& getEventNotAfter() const noexcept;

	void setEventNotBefore(const char* eventNotBefore);
	const std::string& getEventNotBefore() const noexcept;

	void addConnectionFile(const char* value);
	void setUsername(const char* value);
	void setPassword(const char* value);
	void addURL(const char* value);
	const std::vector<common::Server>& getServers() const noexcept;

	static void printUsage();

private:
	std::unique_ptr<Command> command;
	std::string eventType;
	int priority = -1;
	std::vector<std::pair<std::string, std::string>> settings;
	std::string condition;
	bool wait = false;
	std::string taskId;
	std::string signal;
	std::unique_ptr<common::types::State::Type> state;
	std::string eventNotAfter;
	std::string eventNotBefore;

	// connection options
	std::string currentUsername;
	std::string currentPassword;
	std::vector<common::Server> servers;
};

} /* namespace control */
} /* namespace batchelor */

#endif /* BATCHELOR_CONTROL_OPTIONS_H_ */
