#!/bin/sh

cd batchelor-common    ;            tbuild clean install ; cd ..
cd batchelor-condition ; ./run.sh ; tbuild clean install ; cd ..
cd batchelor-service   ;            tbuild clean install ; cd ..
cd batchelor-control   ;            tbuild clean install ; cd ..
cd batchelor-worker    ;            tbuild clean install ; cd ..
cd batchelor-head      ;            tbuild clean install ; cd ..
cd batchelor-ui        ;            tbuild clean install ; cd ..

##rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-batchelor-0.0.1.spec
#toolcontainer rpmbuild --define "_topdir /workspace/rpm" --target x86_64 -bb rpmbuild-batchelor-1.0.0.spec
#mv rpm/RPMS/x86_64/*.rpm .
#rm -rf rpm
