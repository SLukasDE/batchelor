#include <batchelor/condition/Main.h>


int main(int argc, const char *argv[]) {
	batchelor::condition::Main main;

	main.testScanner();
	main.testParser();

	return 0;
}
