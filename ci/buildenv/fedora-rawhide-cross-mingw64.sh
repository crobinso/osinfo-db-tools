# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool manifest ci/manifest.yml
#
# https://gitlab.com/libvirt/libvirt-ci

function install_buildenv() {
    dnf update -y --nogpgcheck fedora-gpg-keys
    dnf distro-sync -y
    dnf install -y \
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
        rpm-build
    rm -f /usr/lib*/python3*/EXTERNALLY-MANAGED
    dnf install -y \
        mingw64-gcc \
        mingw64-gettext \
        mingw64-glib2 \
        mingw64-headers \
        mingw64-json-glib \
        mingw64-libarchive \
        mingw64-libsoup \
        mingw64-libxml2 \
        mingw64-pkg-config
    rpm -qa | sort > /packages.txt
    mkdir -p /usr/libexec/ccache-wrappers
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-cc
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-gcc
}

export CCACHE_WRAPPERSDIR="/usr/libexec/ccache-wrappers"
export LANG="en_US.UTF-8"
export MAKE="/usr/bin/make"
export NINJA="/usr/bin/ninja"
export PYTHON="/usr/bin/python3"

export ABI="x86_64-w64-mingw32"
export MESON_OPTS="--cross-file=/usr/share/mingw/toolchain-mingw64.meson"
