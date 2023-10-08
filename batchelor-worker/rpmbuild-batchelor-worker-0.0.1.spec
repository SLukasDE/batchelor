##################################
# Spec file for Batchelor Worker #
##################################

Summary: Batchelor Worker
Name: batchelor-worker
Version: 0.0.1
Release: 1
License: AGPL
URL: http://www.sven-lukas.de
Group: System
Packager: Sven Lukas
BuildRoot: ./rpmbuild/

%description
Batchelor Worker deamon allowes to execute pre-defined cli tasks remotely.

%prep
#echo "BUILDROOT = $RPM_BUILD_ROOT"

mkdir -p $RPM_BUILD_ROOT/usr/bin
cp -a ../../build/batchelor-worker/0.0.1/default/architecture/linux-gcc/link-executable/batchelor-worker $RPM_BUILD_ROOT/usr/bin

exit

%files
%attr(0755, root, root) /usr/bin/batchelor-worker

%pre

%post

%postun

%clean

%changelog
* Tue Sep 12 2023 Sven Lukas <sven.lukas@gmail.com>
  - First prebuild RPM
