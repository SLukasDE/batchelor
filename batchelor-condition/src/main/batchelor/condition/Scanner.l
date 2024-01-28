%{
/* C++ string header, for string ops below */
#include <cstdlib>
#include <string>
 
/* Implementation of yyFlexScanner */ 
#include <batchelor/condition/Compiler.h>
#include <batchelor/condition/Scanner.h>

namespace batchelor {
namespace condition {

Scanner::Scanner(std::istream& in)
: yyFlexLexer(in, std::cout)
{ };

} /* namespace condition */
} /* namespace batchelor */


/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

#undef  YY_DECL
#define YY_DECL int batchelor::condition::Scanner::fetchNextToken(void* const semanticType, void* location, batchelor::condition::Compiler& compiler)

/* update location on matching */
/*
#define YY_USER_ACTION \
  location->step(); \
  location->columns(yyleng);
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

NUM     [0-9]+(\.[0-9]+)?
STR     \"([][a-zA-Z0-9 !�$%&/()={}_]|\.|(\\\"))*\"

%%

%{          /** Code executed at the beginning of lexer(...) **/
            //semanticType = aSemanticType;
%}

\n          {
               // Update line number
               //location->lines();
               compiler.doSomething();
            }

{NUM}                     {
                            //semanticType->emplace<double>(std::atof(YYText()));
                            return NUMBER;
                          }

{STR}                     {
                            std::string str(YYText());
                            //semanticType->emplace<std::string>(str.substr(1, str.size()-2));
                            return STRING;
                          }

&&                        {
                            return IDENTIFIER_AND;
                          }

\|\|                      {
                            return IDENTIFIER_OR;
                          }

[a-zA-Z][a-zA-Z0-9_]*     {
                            std::string str(YYText());
                            if(str == "proc")
                              return IDENTIFIER_PROC;

                            if(str == "var")
                              return IDENTIFIER_VAR;

                            if(str == "true")
                              return IDENTIFIER_TRUE;

                            if(str == "false")
                              return IDENTIFIER_FALSE;

                            return IDENTIFIER;
                          }


[ \t]                   ;

.                         {
                            //semanticType->emplace<char>(YYText()[0]);
                            //return CHAR;
                            return yytext[0];
                          }



%%


