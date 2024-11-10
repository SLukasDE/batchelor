%skeleton "lalr1.cc"
%require  "3.2"
%defines "Parser.h"
%output "Parser.cpp"
%define api.namespace {batchelor::condition}
%define api.parser.class {Parser}

// This code is copied at the beginning of the generated file Parser.h
%code requires{

#include <batchelor/condition/Function.h>
#include <batchelor/condition/ObjectType.h>
#include <batchelor/condition/Value.h>
#include <batchelor/condition/ValueType.h>

namespace batchelor {
namespace condition {

class Compiler;
class Scanner;

} /* namespace condition */
} /* namespace batchelor */

}

%parse-param { Value& value }
%parse-param { const Compiler& compiler }
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
#include <iostream>

namespace batchelor {
namespace condition {

void Compiler::parse(Scanner& scanner) {
	Parser parser(value, *this, scanner);
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

//%token <char>               CHAR
%token <std::string>        STRING     256
%token <double>             NUMBER     257
%token <std::string>        IDENTIFIER 258
%token                      TRUE       259
%token                      FALSE      260
%token                      EQ         261
%token                      NE         262
%token                      LT         263
%token                      LE         264
%token                      GT         265
%token                      GE         266
%token                      NOT        267
%token                      AND        268
%token                      OR         269
%token                      ADD        270
%token                      SUB        271
%token                      MUL        272
%token                      DIV        273
%token                      VAR_OPEN   274

%type  <Value>              value_term
%type  <Value>              value_or
%type  <Value>              value_and
%type  <Value>              value_cmp
%type  <Value>              value_add
%type  <Value>              value_mul
%type  <Value>              value_not
%type  <Value>              variable
%type  <Value>              function
%type  <std::vector<Value>> args

%%

value:
  value_term {
    value = std::move($1);
  }
;

args:
  value_term {
    $$.emplace_back($1);
  }
|
  args ',' value_term {
    $$ = std::move($1);
    $$.emplace_back($3);
  }
;

variable:
  VAR_OPEN IDENTIFIER '}' {
    $$.objectType = ObjectType::otVariable;
    $$.valueString = $2;
  }
;

function:
  IDENTIFIER '(' ')' {
    const auto& functionType = compiler.getFunction($1);
    if(!functionType.arguments.empty()) {
    	throw std::runtime_error("Function \"" + $1 + "\" called with 0 arguments, but " + std::to_string(functionType.arguments.size())  + " arguments required.");
    }
    $$.objectType = ObjectType::otFunction;
    $$.valueString = $1;
    //$$.args = ;
    $$.functionType = functionType;
  }
|
  IDENTIFIER '(' args ')' {
    const auto& functionType = compiler.getFunction($1);
    if(functionType.arguments.size() != $3.size()) {
    	throw std::runtime_error("Function \"" + $1 + "\" called with " + std::to_string($3.size()) + " arguments, but " + std::to_string(functionType.arguments.size())  + " arguemnts required.");
    }
    $$.objectType = ObjectType::otFunction;
    $$.valueString = $1;
    $$.args = $3;
    $$.functionType = functionType;
  }
;

value_or:
  value_or OR value_and {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "OR";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_and {
    $$ = std::move($1);
  }
;

value_and:
  value_and AND value_cmp {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "AND";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp {
    $$ = std::move($1);
  }
;

value_cmp:
  value_cmp EQ value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
    if($1.getValueType() == ValueType::vtString || $3.getValueType() == ValueType::vtString) {
	    $$.valueString = "EQ_STR";
    }
    else if($1.getValueType() == ValueType::vtNumber || $3.getValueType() == ValueType::vtNumber) {
	    $$.valueString = "EQ_NUM";
    }
    else /* if($1.getValueType() == ValueType::vtBool || $3.getValueType() == ValueType::vtBool) */ {
	    $$.valueString = "EQ_BOOL";
    }
    
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp NE value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
    if($1.getValueType() == ValueType::vtString || $3.getValueType() == ValueType::vtString) {
	    $$.valueString = "NE_STR";
    }
    else if($1.getValueType() == ValueType::vtNumber || $3.getValueType() == ValueType::vtNumber) {
	    $$.valueString = "NE_NUM";
    }
    else /* if($1.getValueType() == ValueType::vtBool || $3.getValueType() == ValueType::vtBool) */ {
	    $$.valueString = "NE_BOOL";
    }
    
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp LT value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "LT";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp LE value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "LE";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp GT value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "GT";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_cmp GE value_add {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "GE";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_add {
    $$ = std::move($1);
  }
;

value_add:
  value_add ADD value_mul {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
    if($1.getValueType() == ValueType::vtBool || $3.getValueType() == ValueType::vtBool) {
	    $$.valueString = "OR";
    }
    else if($1.getValueType() == ValueType::vtNumber || $3.getValueType() == ValueType::vtNumber) {
	    $$.valueString = "ADD_NUM";
    }
    else /* if($1.getValueType() == ValueType::vtString || $3.getValueType() == ValueType::vtString) */ {
	    $$.valueString = "ADD_STR";
    }
    
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_add SUB value_mul {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "SUB";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_mul {
    $$ = std::move($1);
  }
;

value_mul:
  value_mul MUL value_term {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "MUL";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_mul DIV value_term {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "DIV";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($1);
    $$.args.emplace_back($3);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_not {
    $$ = std::move($1);
  }
;

value_not:
  NOT value_term {
    /* ToDo: can be improved by evaluating constant values directly and add convert functions diretly into the AST */
    
	$$.valueString = "NOT";
    $$.objectType = ObjectType::otFunction;
    $$.args.emplace_back($2);
   	$$.functionType = compiler.getFunction($$.valueString);
  }
|
  value_term {
    $$ = std::move($1);
  }
;

value_term:
  STRING {
    $$.objectType = ObjectType::otString;
    $$.valueString = $1;
  }
|
  NUMBER {
    $$.objectType = ObjectType::otNumber;
    $$.valueNumber = $1;
  }
|
  TRUE {
    $$.objectType = ObjectType::otBool;
    $$.valueBool = true;
  }
|
  FALSE {
    $$.objectType = ObjectType::otBool;
    $$.valueBool = false;
  }
|
  variable {
    $$ = std::move($1);
  }
|
  function {
    $$ = std::move($1);
  }
|
  '(' value_or ')' {
    $$ = std::move($2);
  }
;

%%


void batchelor::condition::Parser::error(const location_type& l, const std::string& err_message) {
	std::stringstream s;
	s << "Error: " << err_message << " at " << l;
	throw std::runtime_error(s.str());
}
