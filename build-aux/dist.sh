#!/bin/sh

SOURCE_ROOT=$1
BUILD_ROOT=$2

$SOURCE_ROOT/build-aux/gitlog-to-changelog > $MESON_DIST_ROOT/ChangeLog

# Only osinfo-db-tools.spec is put into the tarball,
# otherwise rpmbuild -ta $tarball.tar.gz would fail
cp $BUILD_ROOT/osinfo-db-tools.spec $MESON_DIST_ROOT/

out="`git log --pretty=format:'%aN <%aE>' | sort -u`"
perl -p -e "s/#authorslist#// and print '$out'" < $SOURCE_ROOT/AUTHORS.in > $MESON_DIST_ROOT/AUTHORS
