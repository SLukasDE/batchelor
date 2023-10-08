#################################
# Spec file for Batchelor Tools #
#################################

Summary: Batchelor Tools
Name: batchelor
Version: 0.0.1
Release: 1
License: AGPL
URL: http://www.sven-lukas.de
Group: System
Packager: Sven Lukas
BuildRoot: ./rpmbuild/

%description
Batchelor allowes to execute pre-defined cli tasks remotely.

%prep
#echo "BUILDROOT = $RPM_BUILD_ROOT"

mkdir -p $RPM_BUILD_ROOT/usr/bin
cp -a ../../batchelor-control/build/batchelor-control/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-control $RPM_BUILD_ROOT/usr/bin
cp -a ../../batchelor-head/build/batchelor-head/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-head $RPM_BUILD_ROOT/usr/bin
cp -a ../../batchelor-worker/build/batchelor-worker/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-worker $RPM_BUILD_ROOT/usr/bin

exit

%files
%attr(0755, root, root) /usr/bin/batchelor-control
%attr(0755, root, root) /usr/bin/batchelor-head
%attr(0755, root, root) /usr/bin/batchelor-worker

%pre

%post

%postun

%clean

%changelog
* Tue Sep 12 2023 Sven Lukas <sven.lukas@gmail.com>
  - First prebuild RPM
