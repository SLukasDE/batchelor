#!/bin/sh

rpmbuild --define "_topdir `pwd`/rpm" --target x86_64 -bb rpmbuild-batchelor-worker-0.0.1.spec
mv rpm/RPMS/x86_64/*.rpm .
rm -rf rpm
