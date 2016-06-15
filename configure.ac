AC_INIT([osinfo-db-tools], [1.0.0], [libosinfo@redhat.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11.1 foreign color-tests tar-ustar])
AC_PREREQ([2.61])
AC_CONFIG_SRCDIR([tools/osinfo-db-validate.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_PROG_LN_S

LT_INIT([shared disable-static win32-dll])

LIBOSINFO_COMPILE_WARNINGS
GNOME_MAINTAINER_MODE_DEFINES
AM_MAINTAINER_MODE([enable])

# i18 support
IT_PROG_INTLTOOL(0.40.0)

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[The name of the gettext domain])

# Autoconf 2.61a.99 and earlier don't support linking a file only
# in VPATH builds.  But since GNUmakefile is for maintainer use
# only, it does not matter if we skip the link with older autoconf.
# Automake 1.10.1 and earlier try to remove GNUmakefile in non-VPATH
# builds, so use a shell variable to bypass this.
GNUmakefile=GNUmakefile
m4_if(m4_version_compare([2.61a.100],
        m4_defn([m4_PACKAGE_VERSION])), [1], [],
      [AC_CONFIG_LINKS([$GNUmakefile:$GNUmakefile], [],
        [GNUmakefile=$GNUmakefile])])


# Use the silent-rules feature when possible.
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Keep these two definitions in agreement.
GLIB_MINIMUM_VERSION="2.36"
GLIB_ENCODED_VERSION="GLIB_VERSION_2_36"

PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= 2.6.0])
PKG_CHECK_MODULES([LIBXSLT], [libxslt >= 1.0.0])
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive >= 3.0.0])

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= $GLIB_MINIMUM_VERSION gobject-2.0 gio-2.0])
GLIB_CFLAGS="$GLIB_CFLAGS -DGLIB_VERSION_MIN_REQUIRED=$GLIB_ENCODED_VERSION"
GLIB_CFLAGS="$GLIB_CFLAGS -DGLIB_VERSION_MAX_ALLOWED=$GLIB_ENCODED_VERSION"
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AC_CONFIG_FILES([
	Makefile
	osinfo-db-tools.spec
	mingw-osinfo-db-tools.spec
	tools/Makefile
	docs/Makefile
	po/Makefile.in
])
AC_OUTPUT
