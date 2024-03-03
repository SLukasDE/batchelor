#!/bin/sh

#rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-batchelor-1.0.0.spec
toolcontainer rpmbuild --define "_topdir /workspace/rpm" --target x86_64 -bb rpmbuild-batchelor-1.0.0.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm
