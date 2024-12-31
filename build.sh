#!/bin/sh

cd batchelor-common    ;            tbuild2 clean install ; cd ..
cd batchelor-condition ; ./run.sh ; tbuild2 clean install ; cd ..
cd batchelor-service   ;            tbuild2 clean install ; cd ..
cd batchelor-control   ;            tbuild2 clean install ; cd ..
cd batchelor-worker    ;            tbuild2 clean install ; cd ..
cd batchelor-head      ;            tbuild2 clean install ; cd ..
cd batchelor-ui        ;            tbuild2 clean install ; cd ..

#rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-batchelor-0.0.1.spec
toolcontainer rpmbuild --define "_topdir /workspace/rpm" --target x86_64 -bb rpmbuild-batchelor-1.0.0.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm
