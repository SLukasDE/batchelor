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

#include <batchelor/condition/Compiler.h>
#include <batchelor/condition/FunctionType.h>
#include <batchelor/condition/Main.h>
#include <batchelor/condition/Parser.h>
#include <batchelor/condition/Scanner.h>

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

	int fetchNextToken(void* const semanticType, void* location, const Compiler& compiler) override {
		switch(state++) {
		case 0:
			return '(';
		case 1:
			return Parser::token_type::NOT;
		case 2:
			return Parser::token_type::TRUE;
		case 3:
			return Parser::token_type::AND;
		case 4:
			return Parser::token_type::FALSE;
		case 5:
			return ')';
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
	std::cout << "TEST Scanner" << std::endl;
	std::cout << "============" << std::endl;

	std::stringstream sstr;
	//sstr << "Foo(\"String\", (true && (! ${VAR} == 10 )))";
	sstr << "(true && false)";

	Scanner scanner(sstr);
	Compiler compiler;

	while(true) {
		int token = scanner.fetchNextToken(nullptr, nullptr, compiler);

		switch(token) {
		case Parser::token_type::STRING:
			std::cout << "STRING" << std::endl;
			break;
		case Parser::token_type::NUMBER:
			std::cout << "NUMBER" << std::endl;
			break;
		case Parser::token_type::IDENTIFIER:
			std::cout << "IDENTIFIER" << std::endl;
			break;
		case Parser::token_type::TRUE:
			std::cout << "TRUE" << std::endl;
			break;
		case Parser::token_type::FALSE:
			std::cout << "FALSE" << std::endl;
			break;
		case Parser::token_type::EQ:
			std::cout << "EQ" << std::endl;
			break;
		case Parser::token_type::NE:
			std::cout << "NE" << std::endl;
			break;
		case Parser::token_type::LT:
			std::cout << "LT" << std::endl;
			break;
		case Parser::token_type::LE:
			std::cout << "LE" << std::endl;
			break;
		case Parser::token_type::GT:
			std::cout << "GT" << std::endl;
			break;
		case Parser::token_type::GE:
			std::cout << "GE" << std::endl;
			break;
		case Parser::token_type::NOT:
			std::cout << "NOT" << std::endl;
			break;
		case Parser::token_type::AND:
			std::cout << "AND" << std::endl;
			break;
		case Parser::token_type::OR:
			std::cout << "OR" << std::endl;
			break;
		case Parser::token_type::ADD:
			std::cout << "ADD" << std::endl;
			break;
		case Parser::token_type::SUB:
			std::cout << "SUB" << std::endl;
			break;
		case Parser::token_type::MUL:
			std::cout << "MUL" << std::endl;
			break;
		case Parser::token_type::DIV:
			std::cout << "DIV" << std::endl;
			break;
		case Parser::token_type::VAR_OPEN:
			std::cout << "VAR_OPEN" << std::endl;
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
	std::cout << "TEST Parser" << std::endl;
	std::cout << "===========" << std::endl;

	TestScanner scanner;
	Compiler compiler;

	compiler.parse(scanner);
}

void Main::testScannerParser1() {
	std::cout << "TEST Scanner + Parser 1" << std::endl;
	std::cout << "=======================" << std::endl;

	std::stringstream sstr;
	//sstr << "Foo(\"String\", (true && (! ${VAR} == 10 )))";
	sstr << "(true || false)";

	Scanner scanner(sstr);
	Compiler compiler;

	Function function;
	function.returnType = ValueType::vtBool;
	function.arguments.clear();
	function.arguments.emplace_back(ValueType::vtBool);
	function.arguments.emplace_back(ValueType::vtBool);
	compiler.addFunction("Foo", function);

	compiler.parse(scanner);
	std::cout << "Result:" << compiler.toString() << std::endl;
	std::cout << "=====================" << std::endl;

}

void Main::testScannerParser2() {
	std::cout << "TEST Scanner + Parser 2" << std::endl;
	std::cout << "=======================" << std::endl;

	std::stringstream sstr;
	sstr << "((${CLOUD_ID} <> \"GCP\") || (${SECONDS_WAITING} == 20))";
	//sstr << "(${CLOUD_ID} <> \"GCP\") || (true)";
	//sstr << "(${CLOUD_ID} <> \"GCP\")";

	Compiler compiler;
	compiler.addVariable("CLOUD_ID", "GCP");
	compiler.addVariable("SECONDS_WAITING", "48");

	try {
		Scanner scanner(sstr);
		compiler.parse(scanner);
	}
	catch(const std::exception& e) {
		std::cout << "Exception occurred while parsing condition \"" << sstr.str() << "\": \"" << e.what() << "\n";
		return;
	}
	catch(...) {
		std::cout << "Exception occurred while parsing condition \"" << sstr.str() << "\".\n";
		return;
	}

	try {
		std::cout << "Result:" << compiler.toString() << std::endl;
	}
	catch(const std::exception& e) {
		std::cout << "Exception occurred while executing condition \"" << sstr.str() << "\": \"" << e.what() << "\n";
		return;
	}
	catch(...) {
		std::cout << "Exception occurred while executing condition \"" << sstr.str() << "\".\n";
		return;
	}

	std::cout << "=====================" << std::endl;
}

} /* namespace condition */
} /* namespace batchelor */
