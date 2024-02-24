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

#ifndef BATCHELOR_CONDITION_COMPILER_H_
#define BATCHELOR_CONDITION_COMPILER_H_

#include <batchelor/condition/Function.h>
#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Value.h>

#include <istream>
#include <map>
#include <string>

namespace batchelor {
namespace condition {

class Compiler {
public:
	Compiler();

	//void parse(std::istream &stream);
	void parse(Scanner& scanner);

	const Value& getValue() const noexcept;
	bool toBool() const;
	double toNumber() const;
	std::string toString() const;

	void addFunction(const std::string& name, const Function& functionType);
	const Function& getFunction(const std::string& name) const;

	void addVariable(const std::string& key, const std::string& value);
	std::string getVariable(const std::string& key) const;

private:
	bool toBool(const Value& value) const;
	static bool toBool(const std::string& str);
	static bool toBool(const double& number);

	double toNumber(const Value& value) const;
	static double toNumber(const std::string& str);
	static double toNumber(bool b);

	std::string toString(const Value& value) const;
	static std::string toString(bool b);
	static std::string toString(const double& number);

	Value callFunction(const Value& value) const;

	std::map<std::string, Function> functions;
	std::map<std::string, std::string> variables;
	Value value;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_COMPILER_H_ */
