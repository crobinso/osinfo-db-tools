# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile centos-8 osinfo-db-tools
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/b098ec6631a85880f818f2dd25c437d509e53680
FROM docker.io/library/centos:8

RUN dnf update -y && \
    dnf install 'dnf-command(config-manager)' -y && \
    dnf config-manager --set-enabled -y powertools && \
    dnf install -y centos-release-advanced-virtualization && \
    dnf install -y epel-release && \
    dnf install -y \
        ca-certificates \
        ccache \
        gcc \
        gettext \
        git \
        glib2-devel \
        glibc-langpack-en \
        json-glib-devel \
        libarchive-devel \
        libsoup-devel \
        libxml2-devel \
        libxslt-devel \
        make \
        ninja-build \
        pkgconfig \
        python3 \
        python3-pip \
        python3-pytest \
        python3-requests \
        python3-setuptools \
        python3-wheel \
        rpm-build && \
    dnf autoremove -y && \
    dnf clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

RUN pip3 install \
         meson==0.54.0

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
