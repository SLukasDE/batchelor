%skeleton "lalr1.cc"
%require  "3.2"
%defines "Parser.h"
%output "Parser.cpp"
%define api.namespace {batchelor::condition}
%define api.parser.class {Parser}

// This code is copied at the beginning of the generated file Parser.h
%code requires{

#include <batchelor/condition/CObject.h>
#include <batchelor/condition/Value.h>

namespace batchelor {
namespace condition {

class Compiler;
class Scanner;

} /* namespace condition */
} /* namespace batchelor */

}

%parse-param { Compiler& compiler }
%parse-param { Scanner& scanner }

// Tells Bison to call yylex (or scanner.fetchNextToken) at the end with an addition argument "batchelor::condition::Compiler&"
%lex-param { Compiler& compiler }

// This code is copied at the beginning of the generated file Parser.cpp
%code{

/* include for all driver functions */
#include <batchelor/condition/Scanner.h>
#include <batchelor/condition/Compiler.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace batchelor {
namespace condition {

void Compiler::parse(Scanner& scanner) {
	Parser parser(*this, scanner);
	if(parser.parse() != 0) {
		throw std::runtime_error("Parse failed!!");
	}
}
	
} /* namespace condition */
} /* namespace batchelor */
   
// Make the parser calling ...->Scanner::fetchNextToken(...) instead of ::yylex(...) 
#undef yylex
#define yylex scanner.fetchNextToken

}

%define api.value.type variant
%define parse.assert

%locations

%start value
//%start programm

//%token <char>        CHAR

%token <std::string> STRING     256
%token <double>      NUMBER     257
%token <std::string> IDENTIFIER 258
%token               TRUE       259
%token               FALSE      260
%token <bool>        EQ         261
%token <bool>        NE         262
%token <bool>        LT         263
%token <bool>        LE         264
%token <bool>        GT         265
%token <bool>        GE         266
%token <bool>        NOT        267
%token <bool>        AND        268
%token <bool>        OR         269
%token               ADD        270
%token <double>      SUB        271
%token <double>      MUL        272
%token <double>      DIV        273
%token               VAR_OPEN   274

%type  <CObject>     programm
%type  <CObjectList> object_list
%type  <CObject>     object
%type  <Value>       value
%type  <Value>       variable
%type  <Value>       function
%type  <std::vector<Value>> args

%type  <double>      number_sum
%type  <double>      number_fac
%type  <double>      number_term

%type  <std::string> string_sum
%type  <std::string> string_term

%type  <bool>        bool_or
%type  <bool>        bool_and
%type  <bool>        bool_not
%type  <bool>        bool_term

%%

programm:
  object_list {
    $$.add($1);
  }
;

object_list:
  /* epsilon */
  {
    //$$ = new std::vector<CObject>();
  }
|
  object_list object {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
;

args:
  value {
    $$.emplace_back($1);
  }
|
  args ',' value {
    $$ = std::move($1);
    $$.emplace_back($3);
  }
;

variable:
  VAR_OPEN IDENTIFIER '}' {
    $$.objectType = ValueType::vtVariable;
    $$.valueVariable.name = $1;
  }
;

function:
  IDENTIFIER '(' ')' {
    const auto& function = compiler.getFunction($1);
    if(!function.arguments.empty()) {
    	throw std::runtime_error("Function \"" + $1 + "\" called with 0 arguments, but " + std::to_string(function.arguments.size())  + " arguemnts required.");
    }
    $$.objectType = ValueType::vtFunction;
    $$.valueFunction.name = $1;
    //$$.valueFunction.args = ;
    $$.valueFunction.function = function.;
  }
|
  IDENTIFIER '(' args ')' {
    const auto& function = compiler.getFunction($1);
    if(!function.arguments.size() != $3.size()) {
    	throw std::runtime_error("Function \"" + $1 + "\" called with " + std::to_string($3.size()) + " arguments, but " + std::to_string(function.arguments.size())  + " arguemnts required.");
    }
    $$.objectType = ValueType::vtFunction;
    $$.valueFunction.name = $1;
    $$.valueFunction.args = $3;
    $$.valueFunction.function = function.;
  }
;

value:
  STRING {
    $$.objectType = Value::vtString;
    $$.valueString = $1;
  }
|
  NUMBER {
    $$.objectType = Value::vtNumber;
    $$.valueNumber = $1;
  }
|
  TRUE {
    $$.objectType = Value::vtBool;
    $$.valueBool = true;
  }
|
  FALSE {
    $$.objectType = Value::vtBool;
    $$.valueBool = false;
  }
|
  variable {
    $$ = std::move($1);
  }
;

object:
  IDENTIFIER '{' object_list '}' {
    $$.type = CObject::otProcedure;
    $$.add($3);
  }
/*
  IDENTIFIER_PROC '{' object_list '}' {
    $$.type = CObject::otProcedure;
    $$.add($3);
  }
|
  IDENTIFIER_PROC STRING ':' IDENTIFIER '{' object_list '}' {
    $$.name = $2;
    $$.type = CObject::otProcedure;
    $$.v_string = $4;
    $$.add($6);
  }
|
  IDENTIFIER_PROC ':' IDENTIFIER '{' object_list '}' {
    $$.type = CObject::otProcedure;
    $$.v_string = $3;
    $$.add($5);
  }
|
  IDENTIFIER_VAR STRING ';' {
    $$.name = $2;
    $$.type = CObject::otVoid;
  }
|
  IDENTIFIER_VAR STRING ':' number_sum ';' {
    $$.name = $2;
    $$.type = CObject::otDouble;
    $$.v_double = $4;
  }
|
  IDENTIFIER_VAR ':' number_sum ';' {
    $$.type = CObject::otDouble;
    $$.v_double = $3;
  }
|
  IDENTIFIER_VAR STRING ':' string_sum ';' {
    $$.name = $2;
    $$.type = CObject::otString;
    $$.v_string = $4;
  }
|
  IDENTIFIER_VAR ':' string_sum ';' {
    $$.type = CObject::otString;
    $$.v_string = $3;
  }
|
  IDENTIFIER_VAR STRING ':' bool_or ';' {
    $$.name = $2;
    $$.type = CObject::otBool;
    $$.v_bool = $4;
  }
|
  IDENTIFIER_VAR ':' bool_or ';' {
    $$.type = CObject::otBool;
    $$.v_bool = $3;
  }
*/
;

number_sum:
  number_sum '+' number_fac {
    $$ = $1 * $3;
  }
|
  number_sum '-' number_fac {
    $$ = $1 * $3;
  }
|
  number_fac {
    $$ = $1;
  }
;

number_fac:
  number_fac MUL number_term {
    $$ = $1 * $3;
  }
|
  number_fac '/' number_term {
    $$ = $1 / $3;
  }
|
  number_term {
    $$ = $1;
  }
;

number_term:
  NUMBER {
    $$ = $1;
  }
|
  '(' number_sum ')' {
    $$ = $2;
  }
;

string_sum:
  string_sum '+' string_term {
    $$ = $1 + $3;
  }
|
  string_term {
    $$ = $1;
  }
;

string_term:
  STRING {
    $$ = $1;
  }
|
  '(' string_sum ')' {
    $$ = $2;
  }
;

bool_or:
  bool_or OR bool_and {
    $$ = $1 || $3;
  }
|
  bool_and {
    $$ = $1;
  }
;

bool_and:
  bool_and AND bool_not {
    $$ = $1 && $3;
  }
|
  bool_not {
    $$ = $1;
  }
;

bool_not:
  '!' bool_not {
    $$ = ! $2;
  }
|
  bool_term {
    $$ = $1;
  }
;

bool_term:
  TRUE {
    $$ = true;
  }
|
  FALSE {
    $$ = false;
  }
|
  '(' bool_or ')' {
    $$ = $2;
  }
;

%%


void batchelor::condition::Parser::error(const location_type& l, const std::string& err_message) {
	std::stringstream s;
	s << "Error: " << err_message << " at " << l;
	throw std::runtime_error(s.str());
}
