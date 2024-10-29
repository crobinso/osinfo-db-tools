project(
    'osinfo-db-tools', 'c',
    version: '1.13.0',
    license: 'GPLv2+',
    meson_version: '>= 0.49.0',
    default_options: [
        'buildtype=debugoptimized',
        'c_std=gnu99',
        'warning_level=1',
    ],
)

compiler = meson.get_compiler('c')
supported_compiler = false
if compiler.get_id() == 'clang'
    supported_compiler = true
elif compiler.get_id() == 'gcc'
    supported_compiler = true
endif
assert(supported_compiler == true, 'osinfo-db-tools requires GCC or CLang')

osinfo_db_tools_prefix = get_option('prefix')

# those directories have to be known by the project
osinfo_db_tools_datadir = join_paths(osinfo_db_tools_prefix, get_option('datadir'))
osinfo_db_tools_localedir = join_paths(osinfo_db_tools_prefix, get_option('localedir'))
osinfo_db_tools_pkgdatadir = join_paths(osinfo_db_tools_datadir, meson.project_name())
osinfo_db_tools_sysconfdir = join_paths(osinfo_db_tools_prefix, get_option('sysconfdir'))

# those directories will have files installed in
osinfo_db_tools_bindir = join_paths(osinfo_db_tools_prefix, get_option('bindir'))
osinfo_db_tools_docdir = join_paths(osinfo_db_tools_datadir, 'doc', meson.project_name())
osinfo_db_tools_licensedir = join_paths(osinfo_db_tools_datadir, 'license', meson.project_name())
osinfo_db_tools_mandir = join_paths(osinfo_db_tools_prefix, get_option('mandir'))

# spec files
osinfo_db_tools_spec_data = configuration_data()
osinfo_db_tools_spec_data.set('VERSION', meson.project_version())

configure_file(
    input: 'osinfo-db-tools.spec.in',
    output: 'osinfo-db-tools.spec',
    configuration: osinfo_db_tools_spec_data
)

# ninja dist helper
meson.add_dist_script('build-aux/dist.sh', meson.source_root(), meson.build_root())

# dependencies
#  glib stuff
glib_version = '2.36'
glib_version_required = 'GLIB_VERSION_2_36'
glib_version_info = '>= @0@'.format(glib_version)
glib_dep = dependency('glib-2.0', version: glib_version_info)
gio_dep = dependency('gio-2.0', version: glib_version_info)
gobject_dep = dependency('gobject-2.0', version: glib_version_info)

#  everything else
json_glib_dep = dependency('json-glib-1.0')
libarchive_dep = dependency('libarchive', version: '>= 3.0.0')
libsoup_dep = dependency('libsoup-3.0', required: false)
#    fallback to libsoup2
if not libsoup_dep.found()
  libsoup_dep = dependency('libsoup-2.4')
endif
libxml_dep = dependency('libxml-2.0', version: '>= 2.6.0')

#  common dependencies
osinfo_db_tools_common_dependencies = [gobject_dep, gio_dep, glib_dep]

# arguments
osinfo_db_tools_cflags = []

#  glib stuff
osinfo_db_tools_cflags += [
    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_36',
    '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_36'
]

#  directories used
osinfo_db_tools_cflags += [
    '-DPKGDATADIR="@0@"'.format(osinfo_db_tools_pkgdatadir),
    '-DDATA_DIR="@0@"'.format(osinfo_db_tools_datadir),
    '-DSYSCONFDIR="@0@"'.format(osinfo_db_tools_sysconfdir),
    '-DLOCALEDIR="@0@"'.format(osinfo_db_tools_localedir),
]

#  gettext package name
osinfo_db_tools_cflags += ['-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name())]

#  cflags to check whether the compiler supports them or not
osinfo_db_tools_check_cflags = [
  '-W',
  '-Waddress',
  '-Waggressive-loop-optimizations',
  '-Wall',
  '-Warray-bounds',
  '-Wattributes',
  '-Wbuiltin-macro-redefined',
  '-Wcast-align',
  '-Wchar-subscripts',
  '-Wclobbered',
  '-Wcomment',
  '-Wcomments',
  '-Wcoverage-mismatch',
  '-Wcpp',
  '-Wdate-time',
  '-Wdeprecated-declarations',
  '-Wdisabled-optimization',
  '-Wdiv-by-zero',
  '-Wdouble-promotion',
  '-Wempty-body',
  '-Wendif-labels',
  '-Wenum-compare',
  '-Wextra',
  '-Wformat-contains-nul',
  '-Wformat-extra-args',
  '-Wformat-security',
  '-Wformat-y2k',
  '-Wformat-zero-length',
  '-Wfree-nonheap-object',
  '-Wignored-qualifiers',
  '-Wimplicit',
  '-Wimplicit-function-declaration',
  '-Wimplicit-int',
  '-Winit-self',
  '-Winline',
  '-Wint-to-pointer-cast',
  '-Winvalid-memory-model',
  '-Winvalid-pch',
  '-Wjump-misses-init',
  '-Wlogical-op',
  '-Wmain',
  '-Wmaybe-uninitialized',
  '-Wmissing-braces',
  '-Wmissing-declarations',
  '-Wmissing-field-initializers',
  '-Wmissing-include-dirs',
  '-Wmissing-parameter-type',
  '-Wmissing-prototypes',
  '-Wmultichar',
  '-Wnarrowing',
  '-Wnested-externs',
  '-Wnonnull',
  '-Wold-style-declaration',
  '-Wold-style-definition',
  '-Wopenmp-simd',
  '-Woverflow',
  '-Woverlength-strings',
  '-Woverride-init',
  '-Wpacked',
  '-Wpacked-bitfield-compat',
  '-Wparentheses',
  '-Wpointer-arith',
  '-Wpointer-sign',
  '-Wpointer-to-int-cast',
  '-Wpragmas',
  '-Wreturn-local-addr',
  '-Wreturn-type',
  '-Wsequence-point',
  '-Wshadow',
  '-Wsizeof-pointer-memaccess',
  '-Wstack-protector',
  '-Wstrict-aliasing',
  '-Wstrict-overflow',
  '-Wstrict-prototypes',
  '-Wsuggest-attribute=const',
  '-Wsuggest-attribute=format',
  '-Wsuggest-attribute=noreturn',
  '-Wsuggest-attribute=pure',
  '-Wswitch',
  '-Wswitch-default',
  '-Wsync-nand',
  '-Wtrampolines',
  '-Wtrigraphs',
  '-Wtype-limits',
  '-Wuninitialized',
  '-Wunknown-pragmas',
  '-Wunsafe-loop-optimizations',
  '-Wunused',
  '-Wunused-but-set-parameter',
  '-Wunused-but-set-variable',
  '-Wunused-function',
  '-Wunused-label',
  '-Wunused-local-typedefs',
  '-Wunused-parameter',
  '-Wunused-result',
  '-Wunused-value',
  '-Wunused-variable',
  '-Wvarargs',
  '-Wvariadic-macros',
  '-Wvector-operation-performance',
  '-Wvla',
  '-Wvolatile-register-var',
  '-Wwrite-strings',
  '-Wnormalized=nfc',
  '-Wno-sign-compare',
  '-Wno-sign-conversion',
  '-Wno-conversion',
  '-Wno-unused-parameter',
  '-Wjump-misses-init',
  '-Wframe-larger-than=4096',
  '-Wno-overlength-strings',
  '--param=ssp-buffer-size=4',
  '-fexceptions',
  '-fasynchronous-unwind-tables',
  '-fdiagnostics-show-option',
  '-funit-at-a-time',
  '-fipa-pure-const',
  '-Wno-suggest-attribute=pure',
  '-Wno-suggest-attribute=const',
]

if host_machine.system() != 'windows'
    osinfo_db_tools_check_cflags += ['-fstack-protector-all']
endif

if run_command('[', '-d', '.git', ']').returncode() == 0
    osinfo_db_tools_check_cflags += ['-Werror']
endif

foreach cflag: osinfo_db_tools_check_cflags
  if compiler.has_argument(cflag)
    osinfo_db_tools_cflags += [cflag]
  endif
endforeach

add_project_arguments(osinfo_db_tools_cflags, language: 'c')

subdir('tools')
subdir('po')
subdir('tests')
