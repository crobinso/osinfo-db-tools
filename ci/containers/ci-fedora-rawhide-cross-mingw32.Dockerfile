FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf update -y --nogpgcheck fedora-gpg-keys && \
    dnf update -y && \
    dnf install -y \
        bash \
        bash-completion \
        ca-certificates \
        ccache \
        gcc \
        git \
        glibc-langpack-en \
        make \
        meson \
        ninja-build \
        patch \
        perl \
        perl-App-cpanminus \
        python3 \
        python3-pip \
        python3-pytest \
        python3-requests \
        python3-setuptools \
        python3-wheel \
        rpm-build && \
    dnf autoremove -y && \
    dnf clean all -y && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-$(basename /usr/bin/gcc)

RUN dnf install -y \
        mingw32-gcc \
        mingw32-gettext \
        mingw32-glib2 \
        mingw32-headers \
        mingw32-json-glib \
        mingw32-libarchive \
        mingw32-libsoup \
        mingw32-libxml2 \
        mingw32-libxslt \
        mingw32-pkg-config && \
    dnf clean all -y

ENV LANG "en_US.UTF-8"

ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"

ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "i686-w64-mingw32"
ENV CONFIGURE_OPTS "--host=i686-w64-mingw32"
ENV MESON_OPTS "--cross-file=/usr/share/mingw/toolchain-mingw32.meson"
