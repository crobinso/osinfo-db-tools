AM_CFLAGS = $(GOBJECT_CFLAGS) \
	    $(GIO_CFLAGS)     \
	    $(GLIB_CFLAGS)    \
	    $(LIBXML_CFLAGS)  \
	    -DPKGDATADIR="\"$(pkgdatadir)\"" \
	    -DLOCALEDIR="\"$(datadir)/locale\"" \
	    $(WARN_CFLAGS) \
	    -I$(top_srcdir) \
            $(NULL)

bin_PROGRAMS = osinfo-db-validate

man1_MANS = osinfo-db-validate.1

CLEANFILES = $(man1_MANS)

POD2MAN = pod2man -c "Virtualization Support" -r "$(PACKAGE)-$(VERSION)"

%.1: %.c Makefile
	$(AM_V_GEN)$(POD2MAN) $< $@

osinfo_db_validate_SOURCES = osinfo-db-validate.c
osinfo_db_validate_LDADD = $(GOBJECT_LIBS)	\
		      $(GIO_LIBS)		\
		      $(GLIB_LIBS)		\
		      $(LIBXML_LIBS)
