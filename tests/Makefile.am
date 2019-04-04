EXTRA_DIST = data

test_helpers = \
	util.py \
	$(NULL)

test_programs = \
	test_osinfo_db_export_import.py \
	test_osinfo_db_path.py \
	test_osinfo_db_validate.py \
	$(NULL)

EXTRA_DIST += \
	$(test_helpers) \
	$(test_programs) \
	$(NULL)

TESTS = \
	$(test_programs) \
	$(NULL)

TESTS_ENVIRONMENT = \
	abs_top_builddir="$(abs_top_builddir)" \
	abs_top_srcdir="$(abs_top_srcdir)" \
	datadir="$(datadir)" \
	sysconfdir="$(sysconfdir)"
