EXTRA_DIST = data

test_helpers = \
	util.py \
	$(NULL)

test_programs = \
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
	abs_top_srcdir="$(abs_top_srcdir)"
