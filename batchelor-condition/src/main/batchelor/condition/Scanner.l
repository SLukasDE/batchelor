%{
/* C++ string header, for string ops below */
#include <cstdlib>
#include <string>
 
/* Implementation of yyFlexScanner */ 
#include <batchelor/condition/Compiler.h>
#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Parser.h>

namespace batchelor {
namespace condition {

namespace {

Parser::semantic_type* toSemanticType(void* const aSemanticType) {
	return static_cast<Parser::semantic_type* const>(aSemanticType);
}

Parser::location_type* toLocationType(void* aLocationType) {
	return static_cast<Parser::location_type*>(aLocationType);
}

} /* anonymous namespace */

Scanner::Scanner(std::istream& in)
: yyFlexLexer(in, std::cout)
{ };

} /* namespace condition */
} /* namespace batchelor */


/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

#undef  YY_DECL
#define YY_DECL int batchelor::condition::Scanner::fetchNextToken(void* const aSemanticType, void* aLocation, const batchelor::condition::Compiler& compiler)

/* update location on matching */
/*
#define YY_USER_ACTION \
	if(location) { \
		location->step(); \
		location->columns(yyleng); \
	}
*/

/* define yyterminate as this instead of NULL */
//#define yyterminate() return( END )

%}

%option debug
%option nodefault
%option yyclass="batchelor::condition::Scanner"
%option yylineno
%option noyywrap
%option c++

STRING					\"([][a-zA-Z0-9 !�$%&/()={}_-]|\.|(\\\"))*\"
NUMBER					[0-9]+(\.[0-9]+)?

%%

%{
/** Code executed at the beginning of lexer(...) **/
auto semanticType = toSemanticType(aSemanticType);
auto location = toLocationType(aLocation);
%}

\n						{
							// Update line number
							if(location) {
								location->lines();
							}
							//compiler.doSomething();
						}

{STRING}				{
							std::string str(YYText());
							if(semanticType) {
								semanticType->emplace<std::string>(str.substr(1, str.size()-2));
							}
							return Parser::token_type::STRING;
						}

{NUMBER}				{
							if(semanticType) {
								semanticType->emplace<double>(std::atof(YYText()));
							}
                            return Parser::token_type::NUMBER;
						}

[a-zA-Z][a-zA-Z0-9_]*	{
							std::string str(YYText());
							if(str == "true") {
								return Parser::token_type::TRUE;
							}
							
							if(str == "false") {
								return Parser::token_type::FALSE;
							}
							
							if(semanticType) {
								semanticType->emplace<std::string>(str);
							}
							return Parser::token_type::IDENTIFIER;
						}

==						{
							return Parser::token_type::EQ;
						}

\<\>					{
							return Parser::token_type::NE;
						}

!=						{
							return Parser::token_type::NE;
						}

\<						{
							return Parser::token_type::LT;
						}

\<=						{
							return Parser::token_type::LE;
						}

\>						{
							return Parser::token_type::GT;
						}

\>=						{
							return Parser::token_type::GE;
						}

!						{
							return Parser::token_type::NOT;
						}

&&						{
							return Parser::token_type::AND;
						}

\|\|					{
							return Parser::token_type::OR;
						}

\+						{
							return Parser::token_type::ADD;
						}

-						{
							return Parser::token_type::SUB;
						}

\*						{
							return Parser::token_type::MUL;
						}

\/						{
							return Parser::token_type::DIV;
						}

$\{						{
							return Parser::token_type::VAR_OPEN;
						}

[ \t]					;

.						{
							//semanticType->emplace<char>(YYText()[0]);
							//return CHAR;
							return yytext[0];
						}

%%


