%{
/* C++ string header, for string ops below */
#include <cstdlib>
#include <string>
 
/* Implementation of yyFlexScanner */ 
#include <batchelor/condition/Driver.h>
#include <batchelor/condition/Scanner.h>

#undef  YY_DECL
#define YY_DECL int batchelor::condition::Scanner::lexer(batchelor::condition::Parser::semantic_type* const aSemanticType, batchelor::condition::Parser::location_type* location, batchelor::condition::Driver& driver)

/* typedef to make the returns for the tokens shorter */
using token = batchelor::condition::Parser::token;

/* define yyterminate as this instead of NULL */
//#define yyterminate() return( token::END )

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
#define YY_USER_ACTION \
  location->step(); \
  location->columns(yyleng);

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
            semanticType = aSemanticType;
%}

\n          {
               // Update line number
               location->lines();
            }

{NUM}                     {
                            semanticType->emplace<double>(std::atof(YYText()));
                            return token::NUMBER;
                          }

{STR}                     {
                            std::string str(YYText());
                            semanticType->emplace<std::string>(str.substr(1, str.size()-2));
                            return token::STRING;
                          }

&&                        {
                            return token::IDENTIFIER_AND;
                          }

\|\|                      {
                            return token::IDENTIFIER_OR;
                          }

[a-zA-Z][a-zA-Z0-9_]*     {
                            std::string str(YYText());
                            if(str == "proc")
                              return token::IDENTIFIER_PROC;

                            if(str == "var")
                              return token::IDENTIFIER_VAR;

                            if(str == "true")
                              return token::IDENTIFIER_TRUE;

                            if(str == "false")
                              return token::IDENTIFIER_FALSE;

                            semanticType->emplace<std::string>(str);
                            semanticType->build<std::string>(yytext);
                            auto identifierType = driver.getIdentifierType(yytext);
                            
                            return identifierType == Driver::IdentifierType::itUnknown ? token::IDENTIFIER : token::IDENTIFIER;
                          }


[ \t]                   ;

.                         {
                            //semanticType->emplace<char>(YYText()[0]);
                            //return token::CHAR;
                            return yytext[0];
                          }



%%


