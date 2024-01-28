#ifndef BATCHELOR_CONDITION_COMPILER_H_
#define BATCHELOR_CONDITION_COMPILER_H_

#include <batchelor/condition/Scanner.h>

#include <istream>

namespace batchelor {
namespace condition {

class Compiler {
public:
	Compiler() = default;

	void parse(std::istream &stream);
	void parse(Scanner& scanner);

	void doSomething();
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_COMPILER_H_ */
