#!/bin/sh

./clean.sh

flex --outfile=src/main/batchelor/condition/Scanner.cpp  src/main/batchelor/condition/Scanner.l
cd src/main/batchelor/condition
bison -d -v Parser.yy
cd ../../../..






