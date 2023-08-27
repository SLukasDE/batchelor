#!/bin/sh

rm -f src/main/batchelor/condition/Lexer.cpp
rm -f src/main/batchelor/condition/location.hh
rm -f src/main/batchelor/condition/Parser.h
rm -f src/main/batchelor/condition/Parser.cpp
rm -f src/main/batchelor/condition/position.hh
rm -f src/main/batchelor/condition/stack.hh


#exit


flex --outfile=src/main/batchelor/condition/Lexer.cpp  src/main/batchelor/condition/Lexer.l
cd src/main/batchelor/condition
bison -d -v Parser.yy
cd ../../../..

#mv src/main/batchelor/condition/Parser.tab.cc src/main/batchelor/condition/Parser.tab.cpp





