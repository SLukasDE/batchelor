#include <batchelor/common/ArgumentsException.h>

#include <batchelor/worker/Options.h>

#include <cstring>
#include <iostream>

namespace batchelor {
namespace worker {
using namespace batchelor::common;

Options::Options(int argc, const char* argv[]) {
	for(int i=1; i<argc; ++i) {
		std::string currentArg(argv[i]);

		if(currentArg == "-s"  || currentArg == "--setting") {
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
}

void Options::setCondition(const char* aCondition) {
	if(!condition.empty()) {
		throw ArgumentsException("Multiple specification of option \"--condition\" is not allowed.");
	}
	if(!aCondition) {
		throw ArgumentsException("Value missing of option \"--condition\".");
	}

	condition = aCondition;
}

const std::string& Options::getCondition() const noexcept {
	return condition;
}

void Options::setMaximumTasksRunning(std::size_t aMaximumTasksRunning) {
	if(maximumTasksRunning > 0) {
		throw ArgumentsException("Multiple specification of option \"--maximum-tasks-running\" is not allowed.");
	}
	if(aMaximumTasksRunning == 0) {
		throw ArgumentsException("Value of option \"--maximum-tasks-running\" must be greater than 0.");
	}

	maximumTasksRunning = aMaximumTasksRunning;
}

std::size_t Options::getMaximumTasksRunning() const noexcept {
	return maximumTasksRunning;
}

void Options::setWait() {
	if(wait) {
		throw ArgumentsException("Multiple specification of option \"--wait\" is not allowed.");
	}

	wait = true;
}

bool Options::getWait() const noexcept {
	return wait;
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

void Options::addMetrics(const char* aKey, const char* aValue) {
	if(!aKey) {
		throw ArgumentsException("Key missing of option \"--metric\".");
	}
	if(!aValue) {
		throw ArgumentsException("Value missing of option \"--metric\".");
	}
	metrics.emplace_back(std::make_pair(std::string(aKey), std::string(aValue)));
}

const std::vector<std::pair<std::string, std::string>>& Options::getMetrics() const noexcept {
	return metrics;
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

	common::Server server;
	server.username = currentUsername;
	server.password = currentPassword;
	server.url = value;

	servers.push_back(server);
}

const std::vector<common::Server>& Options::getServers() const noexcept {
	return servers;
}

void Options::printUsage() {
	std::cout << "batchelor [OPTIONS]...\n";
	std::cout << "\n";
	std::cout << "Usage:\n";
	std::cout << "  batchelor [CONNECTION OPTIONS] [--setting <key> <value>] [--condition <condition>] [--wait]\n";
	std::cout << "\n";
	std::cout << "OPTIONS:\n";
	std::cout << "  -s, --setting          <key> <value>    This option is allowed to be used multible times. The settings are specific to the event type.\n";
	std::cout << "  -c, --condition        <condition>      Formula that specifies if a worker is allowed to process this event.\n";
	std::cout << "  -w, --wait                              Wait for new messages and return with exit code of task.\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "General CONNECTION OPTIONS:\n";
	std::cout << "  -f, --connection-file  <file>           Connection file can contain all of the following connection options, but addition options are still allowed\n";
	std::cout << "  -U, --server-url       <server-url>     At least one server-url must be specified\n";
	std::cout << "  -u, --username         <username>       If <username> is specified, basic-auth is used\n";
	std::cout << "  -p, --password         <password>       If <password> is specified, basic-auth is used\n";
}

} /* namespace worker */
} /* namespace batchelor */
