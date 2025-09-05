# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool manifest ci/manifest.yml
#
# https://gitlab.com/libvirt/libvirt-ci

FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf update -y --nogpgcheck fedora-gpg-keys && \
    dnf install -y nosync && \
    printf '#!/bin/sh\n\
if test -d /usr/lib64\n\
then\n\
    export LD_PRELOAD=/usr/lib64/nosync/nosync.so\n\
else\n\
    export LD_PRELOAD=/usr/lib/nosync/nosync.so\n\
fi\n\
exec "$@"\n' > /usr/bin/nosync && \
    chmod +x /usr/bin/nosync && \
    nosync dnf distro-sync -y && \
    nosync dnf install -y \
               ca-certificates \
               ccache \
               cppi \
               git \
               glibc-langpack-en \
               make \
               meson \
               ninja-build \
               perl-podlators \
               python3 \
               python3-pytest \
               python3-requests \
               rpm-build && \
    nosync dnf autoremove -y && \
    nosync dnf clean all -y && \
    rm -f /usr/lib*/python3*/EXTERNALLY-MANAGED

ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"

RUN nosync dnf install -y \
               mingw32-gcc \
               mingw32-gettext \
               mingw32-glib2 \
               mingw32-headers \
               mingw32-json-glib \
               mingw32-libarchive \
               mingw32-libsoup \
               mingw32-libxml2 \
               mingw32-pkg-config && \
    nosync dnf clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-gcc

ENV ABI "i686-w64-mingw32"
ENV MESON_OPTS "--cross-file=/usr/share/mingw/toolchain-mingw32.meson"
