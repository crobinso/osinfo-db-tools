
SUBDIRS = tools docs po

INTLTOOL_FILES = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	$(NULL)

DISTCLEANFILES =				\
	intltool-extract			\
	intltool-merge				\
	intltool-update				\
	po/.intltool-merge-cache		\
	$(NULL)

EXTRA_DIST = \
  COPYING.LIB \
  osinfo-db-tools.spec \
  osinfo-db-tools.spec.in \
  mingw-osinfo-db-tools.spec.in \
  build-aux/mktempd \
  cfg.mk \
  GNUmakefile \
  maint.mk \
  AUTHORS.in \
  $(INTLTOOL_FILES) \
  $(NULL)

MAINTAINERCLEANFILES =				\
	po/Makefile.in.in			\
	po/Makevars.template			\
	po/Rules-quot				\
	po/boldquot.sed				\
	po/en@boldquot.header			\
	po/en@quot.header			\
	po/insert-header.sin			\
	po/quot.sed				\
	po/remove-potcdate.sin			\
	$(NULL)

ACLOCAL_AMFLAGS = -I m4

dist-hook: gen-ChangeLog gen-AUTHORS

# Generate the ChangeLog file (with all entries since the switch to git)
# and insert it into the directory we're about to use to create a tarball.
.PHONY: gen-ChangeLog gen-AUTHORS
gen-ChangeLog:
	if test -d $(srcdir)/.git; then				\
	  $(top_srcdir)/build-aux/gitlog-to-changelog		\
	    > $(distdir)/cl-t;					\
	  rm -f $(distdir)/ChangeLog;				\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;		\
	fi

gen-AUTHORS:
	$(AM_V_GEN)if test -d $(srcdir)/.git; then                      \
	   out="`cd $(srcdir) && git log --pretty=format:'%aN <%aE>' | sort -u`" && \
	  perl -p -e "s/#authorslist#// and print '$$out'"            \
	     < $(srcdir)/AUTHORS.in > $(distdir)/AUTHORS-tmp &&        \
	   mv -f $(distdir)/AUTHORS-tmp $(distdir)/AUTHORS ;           \
	fi
