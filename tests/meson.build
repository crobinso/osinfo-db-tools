test(
    'syntax-check',
    find_program('@0@/build-aux/syntax-check'.format(meson.source_root())),
    suite: 'style',
    env: [
        'MESON_SOURCE_ROOT=@0@'.format(meson.source_root()),
    ],
)

python = import('python')

python3 = python.find_installation('python3', required: false)
if python3.found()
    tests = {
        'export-import': 'test_osinfo_db_export_import.py',
        'path': 'test_osinfo_db_path.py',
        'validate': 'test_osinfo_db_validate.py'
    }

    env_vars = [
        'abs_top_builddir=' + meson.build_root(),
        'abs_top_srcdir=' + meson.source_root(),
        'datadir=' + osinfo_db_tools_datadir,
        'sysconfdir=' + osinfo_db_tools_sysconfdir
    ]

    foreach name, file: tests
        test(
            name,
            find_program(file),
            suite: 'unit',
            env: env_vars,
        )
    endforeach
endif
