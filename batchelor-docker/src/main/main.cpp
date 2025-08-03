#include <string_view>
#include <iostream>

#include <unistd.h>

void printUsage() {
	std::cout <<
			"OVERVIEW: batchelor invoker\n"
			"\n"
			"USAGE: <cmd> [other args...]\n"
			"<cmd>:\n"
			"  head       invoke the head server to server requests from UI, workers and controller\n"
			"  worker     runs the worker daemon\n"
			"  control    executes the controller CLI tool\n"
			"  ui         starts the UI server\n";
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		std::cout << "Insufficient arguments.\n";
		printUsage();
		return -1;
	}

	std::string_view cmd(argv[1]);
	if(cmd == "help") {
		printUsage();
	}
	else if(cmd == "head") {
		return execv("/bin/batchelor-head", argv+1);
	}
	else if(cmd == "worker") {
		return execv("/bin/batchelor-worker", argv+1);
	}
	else if(cmd == "control") {
		return execv("/bin/batchelor-control", argv+1);
	}
	else if(cmd == "ui") {
		return execv("/bin/batchelor-ui", argv+1);
	}
	else {
		std::cout << "Wrong command.\n";
		printUsage();
		return -1;
	}
	
	return 0;
}
