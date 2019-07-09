#!/bin/sh

set -e
set -v

test -n "$1" && RESULTS=$1 || RESULTS=results.log
INSTALL_ROOT=$HOME/builder

# Make things clean.
test -f Makefile && make -k distclean || :

rm -rf build
mkdir build
cd build

../autogen.sh --prefix=$INSTALL_ROOT \
    --enable-werror

# If the MAKEFLAGS envvar does not yet include a -j option,
# add -jN where N depends on the number of processors.
case $MAKEFLAGS in
  *-j*) ;;
  *) n=$(getconf _NPROCESSORS_ONLN 2> /dev/null)
    test "$n" -gt 0 || n=1
    n=$(expr $n + 1)
    MAKEFLAGS="$MAKEFLAGS -j$n"
    export MAKEFLAGS
    ;;
esac

make
make install

# set -o pipefail is a bashism; this use of exec is the POSIX alternative
exec 3>&1
st=$(
  exec 4>&1 >&3
  { make check syntax-check 2>&1 3>&- 4>&-; echo $? >&4; } | tee "$RESULTS"
)
exec 3>&-
test "$st" = 0

rm -f *.tar.gz
make dist

if [ -f /usr/bin/rpmbuild ]; then
  rpmbuild --nodeps \
     --define "_sourcedir `pwd`" \
     -ba --clean osinfo-db-tools.spec
fi

# Test mingw32 cross-compile
if test -x /usr/bin/i686-w64-mingw32-gcc ; then
  make distclean

  PKG_CONFIG_PATH="$INSTALL_ROOT/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig" \
  CC="i686-w64-mingw32-gcc" \
  ../configure \
    --build=$(uname -m)-pc-linux \
    --host=i686-w64-mingw32 \
    --prefix="$INSTALL_ROOT/i686-w64-mingw32/sys-root/mingw" \
    --enable-werror

  make
  make install

fi

# Test mingw64 cross-compile
if test -x /usr/bin/x86_64-w64-mingw32-gcc ; then
  make distclean

  PKG_CONFIG_PATH="$INSTALL_ROOT/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig" \
  CC="x86_64-w64-mingw32-gcc" \
  ../configure \
    --build=$(uname -m)-pc-linux \
    --host=x86_64-w64-mingw32 \
    --prefix="$INSTALL_ROOT/i686-w64-mingw32/sys-root/mingw" \
    --enable-werror

  make
  make install

fi

if test -x /usr/bin/i686-w64-mingw32-gcc && test -x /usr/bin/x86_64-w64-mingw32-gcc ; then
  if test -f /usr/bin/rpmbuild ; then
    rpmbuild --nodeps \
       --define "_sourcedir `pwd`" \
       -ba --clean mingw-osinfo-db-tools.spec
  fi
fi
