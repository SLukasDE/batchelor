FROM centos:7

USER root

RUN yum install -y centos-release-scl
RUN yum install -y gcc gcc-c++ boost-devel libmicrohttpd-devel libcurl-devel librdkafka-devel unixODBC-devel sqlite-devel gnutls-devel libpthread devtoolset-11 bison rpm-build; yum clean all
COPY *.rpm /var/tmp
RUN rpm -Uhv /var/tmp/*.rpm
COPY tbuild2 /usr/bin/
COPY tbuild2_dryrun /usr/bin/
COPY tbuild2_systemexec /usr/bin/
CMD mkdir /workspace
CMD chmod 777 /workspace
ENV HOME /workspace
WORKDIR /workspace
