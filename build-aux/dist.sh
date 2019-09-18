#!/bin/sh

SOURCE_ROOT=$1
BUILD_ROOT=$2

# Only osinfo-db-tools.spec is put into the tarball,
# otherwise rpmbuild -ta $tarball.tar.gz would fail
cp $BUILD_ROOT/osinfo-db-tools.spec $MESON_DIST_ROOT/
