#ifndef BATCHELOR_CONDITION_SCANNER_H_
#define BATCHELOR_CONDITION_SCANNER_H_

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <batchelor/condition/Parser.h>
//#include <batchelor/condition/location.hh>

#include <istream>

namespace batchelor {
namespace condition {

class Scanner : public yyFlexLexer {
public:
	Scanner(std::istream& in);

	//get rid of override virtual function warning
	//using FlexLexer::yylex;

	/* Calling "flex Lexer.l" should generate the Implementation of this method
	 * but it will generate a Function/Method in Lexer.cpp started with YY_DECL ...
	 *
	 * But you will find as well the folling "#define" in Lexer.l that will make our result as we want:
	 * #define YY_DECL int batchelor::condition::Scanner::lexer(batchelor::condition::Parser::semantic_type* const lval, batchelor::condition::Parser::location_type* loc)
	 *
	 * To make the parser calling Scanner::lexer(...) instead of ::yylex(...) there is another definition in Parser.yy:
	 * #define yylex scanner.lexer
	 */
	int lexer(Parser::semantic_type* const semanticType, Parser::location_type* location, batchelor::condition::Driver& driver);
	// YY_DECL defined in Lexer.l
	// Method body created by flex in Lexer.cpp


private:
	/* yyval ptr */
	Parser::semantic_type* semanticType = nullptr;
};

} /* namespace condition */
} /* namespace batchelor */

#endif /* BATCHELOR_CONDITION_SCANNER_H_ */
