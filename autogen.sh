#!/bin/sh
# Run this to generate all the initial makefiles, etc.
test -n "$srcdir" || srcdir=$(dirname "$0")
test -n "$srcdir" || srcdir=.

olddir=$(pwd)

cd "$srcdir"

(test -f tools/osinfo-db-validate.c) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level osinfo-db-tools directory"
    exit 1
}

# Real ChangeLog/AUTHORS is auto-generated from GIT logs at
# make dist time, but automake requires that it
# exists at all times :-(
touch ChangeLog AUTHORS

aclocal --install || exit 1
autoreconf --verbose --force --install || exit 1

cd "$olddir"

if [ "$NOCONFIGURE" = "" ]; then
        $srcdir/configure "$@" || exit 1

        if [ "$1" = "--help" ]; then
                exit 0
        else
                echo "Now type 'make' to compile $PKG_NAME" || exit 1
        fi
else
        echo "Skipping configure process."
fi
