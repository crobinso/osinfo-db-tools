#!/bin/sh

set -e
set -v

BUILD_ROOT="$PWD/build"
INSTALL_ROOT="$PWD/install"
RPMBUILD_ROOT="$PWD/rpmbuild"
DIST_ROOT="$BUILD_ROOT/native/meson-dist"

# Make things clean.
rm -rf "$BUILD_ROOT" "$INSTALL_ROOT" "$RPMBUILD_ROOT"

meson "$BUILD_ROOT/native" --werror

ninja -C "$BUILD_ROOT/native"
DESTDIR="$INSTALL_ROOT/native" ninja -C "$BUILD_ROOT/native" install
ninja -C "$BUILD_ROOT/native" test
ninja -C "$BUILD_ROOT/native" dist

if test -x /usr/bin/rpmbuild; then
  rpmbuild --nodeps \
     --define "_topdir $RPMBUILD_ROOT" \
     --define "_sourcedir $DIST_ROOT" \
     -ba --clean "$BUILD_ROOT/native/osinfo-db-tools.spec"
fi

# Test mingw32 cross-compile
if test -x /usr/bin/i686-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw32.meson ; then
  meson "$BUILD_ROOT/win32" \
        --werror \
        --cross-file="/usr/share/mingw/toolchain-mingw32.meson"

  ninja -C "$BUILD_ROOT/win32"
  DESTDIR="$INSTALL_ROOT/win32" ninja -C "$BUILD_ROOT/win32" install
fi

# Test mingw64 cross-compile
if test -x /usr/bin/x86_64-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw64.meson ; then
  meson "$BUILD_ROOT/win64" \
        --werror \
        --cross-file="/usr/share/mingw/toolchain-mingw64.meson"

  ninja -C "$BUILD_ROOT/win64"
  DESTDIR="$INSTALL_ROOT/win64" ninja -C "$BUILD_ROOT/win64" install
fi

mv "$DIST_ROOT"/*.tar.xz .
