#ifndef BATCHELOR_CONDITION_COMPILER_H_
#define BATCHELOR_CONDITION_COMPILER_H_

#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Function.h>

#include <istream>
#include <map>
#include <string>

namespace batchelor {
namespace condition {

class Compiler {
public:
	Compiler() = default;

	void parse(std::istream &stream);
	void parse(Scanner& scanner);

	void addFunction(const Function& function);
	const Function& getFunction(const std::string& name) const;

	void doSomething();

private:
	std::map<std::string, Value::Type> functions;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_COMPILER_H_ */
