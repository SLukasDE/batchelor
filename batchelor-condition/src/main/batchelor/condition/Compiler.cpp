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
#include <batchelor/condition/ObjectType.h>
#include <batchelor/condition/ValueType.h>

#include <stdexcept>

namespace batchelor {
namespace condition {

namespace {
}

Compiler::Compiler()
{
	//Parser parser;
	//Parser::token_type::
	Function function;

	function.returnType = ValueType::vtNumber;
	function.arguments.clear();
	function.arguments.emplace_back(ValueType::vtNumber);
	function.arguments.emplace_back(ValueType::vtNumber);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otNumber;
		rv.valueNumber = toNumber(args.at(0)) + toNumber(args.at(1));

		return rv;
	};
	addFunction("ADD_NUM", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otNumber;
		rv.valueNumber = toNumber(args.at(0)) - toNumber(args.at(1));

		return rv;
	};
	addFunction("SUB",     function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otNumber;
		rv.valueNumber = toNumber(args.at(0)) * toNumber(args.at(1));

		return rv;
	};
	addFunction("MUL",     function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otNumber;
		rv.valueNumber = toNumber(args.at(0)) / toNumber(args.at(1));

		return rv;
	};
	addFunction("DIV",     function);

	function.returnType = ValueType::vtBool;

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) == toNumber(args.at(1));

		return rv;
	};
	addFunction("EQ_NUM",  function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) != toNumber(args.at(1));

		return rv;
	};
	addFunction("NE_NUM",  function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) < toNumber(args.at(1));

		return rv;
	};
	addFunction("LT",      function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) <= toNumber(args.at(1));

		return rv;
	};
	addFunction("LE",      function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) > toNumber(args.at(1));

		return rv;
	};
	addFunction("GT",      function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toNumber(args.at(0)) >= toNumber(args.at(1));

		return rv;
	};
	addFunction("GE",      function);

	function.returnType = ValueType::vtString;
	function.arguments.clear();
	function.arguments.emplace_back(ValueType::vtString);
	function.arguments.emplace_back(ValueType::vtString);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otString;
		rv.valueString = toString(args.at(0)) + toString(args.at(1));

		return rv;
	};
	addFunction("ADD_STR", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toString(args.at(0)) == toString(args.at(1));

		return rv;
	};
	addFunction("EQ_STR", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toString(args.at(0)) != toString(args.at(1));

		return rv;
	};
	addFunction("NE_STR", function);

	function.returnType = ValueType::vtBool;
	function.arguments.clear();
	function.arguments.emplace_back(ValueType::vtBool);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = !toBool(args.at(0));

		return rv;
	};
	addFunction("NOT", function);

	function.returnType = ValueType::vtBool;
	function.arguments.clear();
	function.arguments.emplace_back(ValueType::vtBool);
	function.arguments.emplace_back(ValueType::vtBool);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toBool(args.at(0)) == toBool(args.at(1));

		return rv;
	};
	addFunction("EQ_BOOL", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toBool(args.at(0)) != toBool(args.at(1));

		return rv;
	};
	addFunction("NE_BOOL", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toBool(args.at(0));
		if(rv.valueBool == true) {
			rv.valueBool = toBool(args.at(1));
		}

		return rv;
	};
	addFunction("AND", function);

	function.function = [this](const std::vector<Value>& args) {
		Value rv;

		rv.objectType = ObjectType::otBool;
		rv.valueBool = toBool(args.at(0));
		if(rv.valueBool == false) {
			rv.valueBool = toBool(args.at(1));
		}

		return rv;
	};
	addFunction("OR", function);

}

const Value& Compiler::getValue() const noexcept {
	return value;
}

bool Compiler::toBool() const {
	return toBool(value);
}

double Compiler::toNumber() const {
	return toNumber(value);
}

std::string Compiler::toString() const {
	return toString(value);
}


void Compiler::addFunction(const std::string& name, const Function& function) {
	functions[name] = function;
}

const Function& Compiler::getFunction(const std::string& name) const {
	auto iter = functions.find(name);
	if(iter == functions.end()) {
		throw std::runtime_error("Cannot return unknown function '" + name + "'");
	}
	return iter->second;
}

void Compiler::addVariable(const std::string& key, const std::string& value) {
	variables[key] = value;
}

std::string Compiler::getVariable(const std::string& key) const {
	auto iter = variables.find(key);
	if(iter == variables.end()) {
		throw std::runtime_error("Cannot return unknown variable '" + key + "'");
	}
	return iter->second;
}

bool Compiler::toBool(const Value& value) const {
	switch(value.objectType) {
	case ObjectType::otBool:
		return value.valueBool;
	case ObjectType::otNumber:
		return toBool(value.valueNumber);
	case ObjectType::otString:
		return toBool(value.valueString);
	case ObjectType::otVariable:
		return toBool(getVariable(value.valueString));
	case ObjectType::otFunction:
		return toBool(callFunction(value));
	}
	return false;
}

bool Compiler::toBool(const std::string& str) {
	if(str == "true" || str == "1") {
		return true;
	}
	if(str == "false" || str == "0" || str == "") {
		return false;
	}
	throw std::runtime_error("Cannot convert string '" + str + "' to bool.");
}

bool Compiler::toBool(const double& number) {
	if(number >= 0.999 && number <= 1.001) {
		return true;
	}
	if(number >= -0.001 && number <= 0.001) {
		return false;
	}
	throw std::runtime_error("Cannot convert number '" + std::to_string(number) + "' to bool.");
}

double Compiler::toNumber(const Value& value) const {
	switch(value.objectType) {
	case ObjectType::otBool:
		return toNumber(value.valueBool);
	case ObjectType::otNumber:
		return value.valueNumber;
	case ObjectType::otString:
		return toNumber(value.valueString);
	case ObjectType::otVariable:
		return toNumber(getVariable(value.valueString));
	case ObjectType::otFunction:
		return toNumber(callFunction(value));
	}
	return 0;
}

double Compiler::toNumber(const std::string& str) {
	return std::stod(str);
}

double Compiler::toNumber(bool b) {
	return b ? 1 : 0;
}

std::string Compiler::toString(const Value& value) const {
	switch(value.objectType) {
	case ObjectType::otBool:
		return toString(value.valueBool);
	case ObjectType::otNumber:
		return toString(value.valueNumber);
	case ObjectType::otString:
		return value.valueString;
	case ObjectType::otVariable:
		return toString(getVariable(value.valueString));
	case ObjectType::otFunction:
		return toString(callFunction(value));
	}
	return 0;
}

std::string Compiler::toString(bool b) {
	return b ? "true" : "false";
}

std::string Compiler::toString(const double& number) {
	return std::to_string(number);
}

Value Compiler::callFunction(const Value& value) const {
	if(value.objectType != ObjectType::otFunction) {
    	throw std::runtime_error("Execute error: Cannot call function of value that is not a function.");
	}

	const Function& function = getFunction(value.valueString);

	if(value.args.size() != function.arguments.size()) {
    	throw std::runtime_error("Execute error: Function \"" + value.valueString + "\" called with 0 arguments, but " + std::to_string(function.arguments.size())  + " arguments required.");
	}

	std::vector<Value> args;
	for(std::size_t i=0; i<value.args.size(); ++i) {
		switch(function.arguments[i]) {
		case ValueType::vtBool:
			args.emplace_back(toBool(value.args[i]));
			break;
		case ValueType::vtNumber:
			args.emplace_back(toNumber(value.args[i]));
			break;
		case ValueType::vtString:
			args.emplace_back(toString(value.args[i]));
			break;
		}
	}

	return function.function(args);
}

} /* namespace condition */
} /* namespace batchelor */
