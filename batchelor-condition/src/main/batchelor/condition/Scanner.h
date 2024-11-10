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

#ifndef BATCHELOR_CONDITION_SCANNER_H_
#define BATCHELOR_CONDITION_SCANNER_H_

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <istream>

namespace batchelor {
namespace condition {

class Compiler;

class Scanner : public yyFlexLexer {
public:
	Scanner(std::istream& in);
	virtual ~Scanner() = default;

	//get rid of override virtual function warning
	//using FlexLexer::yylex;

	/* Calling "flex Scanner.l" should generate the Implementation of this method
	 * but it will generate a Function/Method in Scanner.cpp started with YY_DECL ...
	 *
	 * Thus, you will find as well the following "#define" in Scanner.l that will make our result as we want:
     * #define YY_DECL int batchelor::parser::Scanner::fetchNextToken(batchelor::parser::Compiler& compiler)
	 * //#define YY_DECL int batchelor::parser::Scanner::fetchNextToken(batchelor::parser::Parser::semantic_type* const lval, batchelor::parser::Parser::location_type* loc, batchelor::parser::Compiler& compiler)
	 *
	 * To make the parser calling Scanner::fetchNextToken(...) instead of ::yylex(...) there is another definition in Parser.yy:
	 * #define yylex scanner.fetchNextToken
	 */
	virtual int fetchNextToken(void* const semanticType, void* location, const Compiler& compiler);
	/*
	int Scanner::fetchNextToken(void* const aSemanticType, void* aLocation, Compiler& compiler) {
		Parser::semantic_type* const semanticType = static_cast<Parser::semantic_type* const>(aSemanticType);
		Parser::location_type* location = static_cast<Parser::location_type*>(aLocation);
	}
	 */

private:
	/* yyval ptr */
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_SCANNER_H_ */
