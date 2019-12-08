# includes
osinfo_db_tools_include = [include_directories('.')]

# sources
#  common sources
osinfo_db_tools_common_sources = [
    'osinfo-db-util.c',
    'osinfo-db-util.h'
]

# osinfo-db-validate
osinfo_db_validate_sources = [
    osinfo_db_tools_common_sources,
    'osinfo-db-validate.c'
]
osinfo_db_validate_dependencies = [
    osinfo_db_tools_common_dependencies,
    libxml_dep
]
executable(
    'osinfo-db-validate',
    sources: osinfo_db_validate_sources,
    include_directories: osinfo_db_tools_include,
    dependencies: osinfo_db_validate_dependencies,
    install: true
)

# osinfo-db-import
osinfo_db_import_sources = [
    osinfo_db_tools_common_sources,
    'osinfo-db-import.c'
]
osinfo_db_import_dependencies = [
    osinfo_db_tools_common_dependencies,
    json_glib_dep,
    libarchive_dep,
    libsoup_dep
]
executable(
    'osinfo-db-import',
    sources: osinfo_db_import_sources,
    include_directories: osinfo_db_tools_include,
    dependencies: osinfo_db_import_dependencies,
    install: true
)

# osinfo-db-export
osinfo_db_export_sources = [
    osinfo_db_tools_common_sources,
    'osinfo-db-export.c'
]
osinfo_db_export_dependencies = [
    osinfo_db_tools_common_dependencies,
    libarchive_dep
]
executable(
    'osinfo-db-export',
    sources: osinfo_db_export_sources,
    include_directories: osinfo_db_tools_include,
    dependencies: osinfo_db_export_dependencies,
    install: true)

# osinfo-db-path
osinfo_db_path_sources = [
    osinfo_db_tools_common_sources,
    'osinfo-db-path.c'
]
osinfo_db_path_dependencies = [osinfo_db_tools_common_dependencies]
executable(
    'osinfo-db-path',
    sources: osinfo_db_path_sources,
    include_directories: osinfo_db_tools_include,
    dependencies: osinfo_db_path_dependencies,
    install: true
)

# man pages
pod2man = find_program('pod2man')
if pod2man.found()
    files = [
        'osinfo-db-validate',
        'osinfo-db-import',
        'osinfo-db-export',
        'osinfo-db-path'
    ]

    foreach file: files
        custom_target(
            file + '.1',
            output: file + '.1',
            input: file + '.c',
            install: true,
            install_dir: join_paths(osinfo_db_tools_mandir, 'man1'),
            build_by_default: true,
            command: [
                pod2man,
                '-c', 'Osinfo DB Tools',
                '-r', meson.project_name() + '-' + meson.project_version(),
                '--name', file,
                '@INPUT@', '@OUTPUT@']
        )
    endforeach
endif
