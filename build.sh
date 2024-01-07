#!/bin/sh

cd batchelor-common  ;          tbuild2 clean install ; cd ..
cd batchelor-service ;          tbuild2 clean install ; cd ..
cd batchelor-control ;          tbuild2 clean install ; cd ..
cd batchelor-worker  ;          tbuild2 clean install ; cd ..
cd batchelor-head    ;          tbuild2 clean install ; cd ..
