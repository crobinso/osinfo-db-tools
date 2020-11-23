FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf update -y --nogpgcheck fedora-gpg-keys && \
    dnf update -y && \
    dnf install -y \
        ca-certificates \
        ccache \
        gcc \
        git \
        glibc-langpack-en \
        make \
        meson \
        ninja-build \
        python3 \
        python3-pytest \
        python3-requests \
        rpm-build && \
    dnf autoremove -y && \
    dnf clean all -y && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-$(basename /usr/bin/gcc)

RUN dnf install -y \
        mingw64-gcc \
        mingw64-gettext \
        mingw64-glib2 \
        mingw64-json-glib \
        mingw64-libarchive \
        mingw64-libsoup \
        mingw64-libxml2 \
        mingw64-libxslt \
        mingw64-pkg-config && \
    dnf clean all -y

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "x86_64-w64-mingw32"
ENV CONFIGURE_OPTS "--host=x86_64-w64-mingw32"
ENV MESON_OPTS "--cross-file=/usr/share/mingw/toolchain-mingw64.meson"
