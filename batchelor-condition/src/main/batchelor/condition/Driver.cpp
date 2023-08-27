#include <batchelor/condition/Driver.h>

#include <cassert>
#include <cctype>
#include <fstream>
#include <stdexcept>

namespace batchelor {
namespace condition {


void Driver::parse(std::istream &stream) {
	if(!stream.good() && stream.eof()) {
		return;
	}

	try {
		scanner.reset(new Scanner(stream));
	}
	catch(std::bad_alloc &ba) {
		throw std::runtime_error(std::string("Failed to allocate scanner: ") + ba.what());
	}

	try {
		parser.reset(new Parser(*scanner, *this));
	}
	catch(std::bad_alloc &ba) {
		throw std::runtime_error(std::string("Failed to allocate parser: ") + ba.what());
	}

	if(parser->parse() != 0) {
		throw std::runtime_error("Parse failed!!");
	}
}

Driver::IdentifierType Driver::getIdentifierType(const std::string& identifier) const {
	return IdentifierType::itUnknown;
}

void Driver::addSomething() {
}

std::ostream& Driver::print(std::ostream &stream) {
	stream << norm  << "norm" << norm << "\n";
	stream << red   << "red"  << norm << "\n";
	stream << blue  << "blue" << norm << "\n";
	stream << norm  << "norm" << norm << "\n";

	return(stream);
}

} /* namespace condition */
} /* namespace batchelor */
