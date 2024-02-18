#include <batchelor/condition/Main.h>
#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Compiler.h>

#include <iostream>

#include <sstream>

namespace batchelor {
namespace condition {
namespace {


class TestScanner : public Scanner
{
public:
	TestScanner()
	: Scanner(fooStream)
	{ }

	int fetchNextToken(void* const semanticType, void* location, Compiler& compiler) override {
		switch(state++) {
		case 0:
			return 1;
		case 1:
			return 1;
		default:
			break;
		}
		return 0;
	}

	int state = 0;
	static std::stringstream fooStream;
};

std::stringstream TestScanner::fooStream;


} /* anonymous namespace */

Main::Main() {
}

void Main::testScanner() {
	std::stringstream sstr;
	sstr << "Bla \"String\" AND && true    var";


	Scanner scanner(sstr);
	Compiler compiler;

	while(true) {
		int token = scanner.fetchNextToken(nullptr, nullptr, compiler);

		switch(token) {
		case Scanner::IDENTIFIER:
			std::cout << "IDENTIFIER" << std::endl;
			break;
		case Scanner::IDENTIFIER_TRUE:
			std::cout << "IDENTIFIER_TRUE" << std::endl;
			break;
		case Scanner::IDENTIFIER_FALSE:
			std::cout << "IDENTIFIER_FALSE" << std::endl;
			break;
		case Scanner::IDENTIFIER_VAR:
			std::cout << "IDENTIFIER_VAR" << std::endl;
			break;
		case Scanner::IDENTIFIER_PROC:
			std::cout << "IDENTIFIER_PROC" << std::endl;
			break;
		case Scanner::IDENTIFIER_AND:
			std::cout << "IDENTIFIER_AND" << std::endl;
			break;
		case Scanner::IDENTIFIER_OR:
			std::cout << "IDENTIFIER_OR" << std::endl;
			break;
		case Scanner::STRING:
			std::cout << "STRING" << std::endl;
			break;
		case Scanner::NUMBER:
			std::cout << "NUMBER" << std::endl;
			break;
		default:
			std::cout << "Value " << std::to_string(token) << std::endl;
			if(token == 0) {
				return;
			}
		}
	}

}

void Main::testParser() {
	TestScanner scanner;
	Compiler compiler;

	compiler.parse(scanner);
}

} /* namespace condition */
} /* namespace batchelor */
