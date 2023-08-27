#include <batchelor/condition/Driver.h>

#include <iostream>


int main(int argc, const char *argv[]) {
	batchelor::condition::Driver driver;

	driver.print(std::cout);
	driver.print(std::cerr);

	return 0;
}

