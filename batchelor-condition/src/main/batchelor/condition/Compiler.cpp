#include <batchelor/condition/Compiler.h>

#include <stdexcept>

namespace batchelor {
namespace condition {

void Compiler::addFunction(const Function& function) {
	functions[function.name] = function;
}

const Function& Compiler::getFunction(const std::string& name) const {
	auto iter = functions.find(name);
	if(iter == functions.end()) {
		throw std::runtime_error("Cannot return unknown function '" + name + "'");
	}
	return iter->second;
}

void Compiler::doSomething() {

}

} /* namespace condition */
} /* namespace batchelor */
