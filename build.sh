#!/bin/sh

cd batchelor-service/
tbuild2 clean install

cd ../batchelor-common/
tbuild2 clean install

cd ../batchelor-control/
tbuild2 clean install

cd ../batchelor-control-cli/
tbuild2 clean install

cd ../batchelor-worker/
tbuild2 clean install

cd ../batchelor-worker-cli/
tbuild2 clean install

cd ../batchelor-head/
tbuild2 clean install

cd ../batchelor-head-cli/
tbuild2 clean install
toolcontainer g++ -o build/batchelor-head-cli/1.5.0/default/architecture/linux-gcc/link-executable/batchelor-head-cli build/batchelor-head-cli/1.5.0/default/architecture/linux-gcc/compile/main.o build/batchelor-head-cli/1.5.0/default/architecture/linux-gcc/compile/batchelor/head/cli/Main.o build/batchelor-head/1.5.0/default/architecture/linux-gcc/link-static/libbatchelor-head.a build/eslx/1.5.1/variants/debug/architecture/linux-gcc/link-static/libeslx.a build/batchelor-common/1.5.0/default/architecture/linux-gcc/link-static/libbatchelor-common.a build/batchelor-service/1.5.0/default/architecture/linux-gcc/link-static/libbatchelor-service.a build/sergut/1.0.0/default/architecture/linux-gcc/link-static/libsergut.a build/rapidjson/1.0.0/default/architecture/linux-gcc/link-static/librapidjson.a build/tinyxml/1.0.0/default/architecture/linux-gcc/link-static/libtinyxml.a build/eslx/1.5.1/variants/debug/architecture/linux-gcc/link-static/libeslx.a -L/usr/lib64 -lboost_filesystem -lboost_system -lcurl -ldl -lgnutls -lmicrohttpd -lodbc -lpthread -lrdkafka -lsqlite3

cd ..


