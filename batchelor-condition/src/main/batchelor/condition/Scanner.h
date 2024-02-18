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
	//virtual int fetchNextToken(Compiler& compiler);
	virtual int fetchNextToken(void* const semanticType, void* location, Compiler& Compiler);
	/*
	int Scanner::fetchNextToken(void* const aSemanticType, void* aLocation, Compiler& Compiler) {
		Parser::semantic_type* const semanticType = static_cast<Parser::semantic_type* const>(aSemanticType);
		Parser::location_type* location = static_cast<Parser::location_type*>(aLocation);
	}
	 */

	static const int STRING = 256;
	static const int NUMBER = 257;
	static const int IDENTIFIER = 258;
	static const int TRUE = 259;
	static const int FALSE = 260;
	static const int EQ = 261;
	static const int NE = 262;
	static const int LT = 263;
	static const int LE = 264;
	static const int GT = 265;
	static const int GE = 266;
	static const int NOT = 267;
	static const int AND = 268;
	static const int OR = 269;
	static const int ADD = 270;
	static const int SUB = 271;
	static const int MUL = 272;
	static const int DIV = 273;
	static const int VAR_OPEN = 274;

private:
	/* yyval ptr */
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_SCANNER_H_ */
