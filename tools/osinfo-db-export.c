/*
 * Copyright (C) 2016 Red Hat, Inc
 *
 * osinfo-db-export: export a database archive
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#include <locale.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <archive.h>
#include <archive_entry.h>

#include "osinfo-db-util.h"

const char *argv0;

time_t entryts;

static int osinfo_db_export_create_file(const gchar *prefix,
                                        GFile *file,
                                        GFileInfo *info,
                                        GFile *base,
                                        const gchar *target,
                                        struct archive *arc,
                                        gboolean verbose);


static int osinfo_db_export_create_reg(GFile *file,
                                       const gchar *abspath,
                                       const gchar *target,
                                       struct archive *arc)
{
    GFileInputStream *is = NULL;
    GError *err = NULL;
    gchar *buf = NULL;
    gsize size;
    int ret = -1;
    gsize rv;

    is = g_file_read(file, NULL, &err);
    if (!is) {
        g_printerr("%s: cannot read file %s: %s\n",
                   argv0, abspath, err->message);
        goto cleanup;
    }

    size = 64 * 1024;
    buf = g_new0(char, size);
    while (1) {
        rv = g_input_stream_read(G_INPUT_STREAM(is),
                                 buf,
                                 size,
                                 NULL,
                                 &err);
        if (rv == -1) {
            g_printerr("%s: cannot read data %s: %s\n",
                       argv0, abspath, err->message);
            goto cleanup;
        }

        if (rv == 0)
            break;

        if (archive_write_data(arc, buf, rv) < 0) {
            g_printerr("%s: cannot write archive data for %s to %s: %s\n",
                       argv0, abspath, target, archive_error_string(arc));
            goto cleanup;
        }
    }

    ret = 0;
 cleanup:
    if (err)
        g_error_free(err);
    g_object_unref(is);
    g_free(buf);
    return ret;
}

static int osinfo_db_export_create_dir(const gchar *prefix,
                                       GFile *file,
                                       GFile *base,
                                       const gchar *abspath,
                                       const gchar *target,
                                       struct archive *arc,
                                       gboolean verbose)
{
    GFileEnumerator *children;
    GError *err = NULL;
    int ret = -1;

    children = g_file_enumerate_children(file,
                                         G_FILE_ATTRIBUTE_STANDARD_NAME
                                         ","
                                         G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                         G_FILE_QUERY_INFO_NONE,
                                         NULL,
                                         &err);
    if (!children) {
        g_printerr("%s: cannot read directory %s: %s\n",
                   argv0, abspath, err->message);
        goto cleanup;
    }

    while (1) {
        GFileInfo *childinfo = NULL;
        GFile *child = NULL;

        childinfo = g_file_enumerator_next_file(children, NULL, &err);
        if (!childinfo) {
            if (err) {
                g_printerr("%s: cannot read directory entry %s: %s\n",
                           argv0, abspath, err->message);
                goto cleanup;
            } else {
                break;
            }
        }

        child = g_file_enumerator_get_child(children, childinfo);

        if (osinfo_db_export_create_file(prefix, child, childinfo, base, target, arc, verbose) < 0) {
            g_object_unref(child);
            g_object_unref(childinfo);
            goto cleanup;
        }

        g_object_unref(child);
        g_object_unref(childinfo);
    }

    ret = 0;
 cleanup:
    if (err)
        g_error_free(err);
    return ret;
}


static int osinfo_db_export_create_file(const gchar *prefix,
                                        GFile *file,
                                        GFileInfo *info,
                                        GFile *base,
                                        const gchar *target,
                                        struct archive *arc,
                                        gboolean verbose)
{
    GFileType type = g_file_query_file_type(file,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL);
    gchar *abspath, *relpath;
    int ret = -1;
    GError *err = NULL;
    gchar *entpath = NULL;
    struct archive_entry *entry = NULL;

    abspath = g_file_get_path(file);
    relpath = g_file_get_relative_path(base, file);

    if (!info) {
        info = g_file_query_info(file,
                                 G_FILE_ATTRIBUTE_STANDARD_NAME
                                 ","
                                 G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                 G_FILE_QUERY_INFO_NONE,
                                 NULL,
                                 &err);
    } else {
        g_object_ref(info);
    }
    if (!info) {
        g_printerr("%s: cannot get file info %s: %s\n",
                   argv0, abspath, err->message);
        goto cleanup;
    }

    entpath = g_strdup_printf("%s/%s", prefix, relpath ? relpath : "");

    entry = archive_entry_new();
    archive_entry_set_pathname(entry, entpath);

    archive_entry_set_atime(entry, entryts, 0);
    archive_entry_set_ctime(entry, entryts, 0);
    archive_entry_set_mtime(entry, entryts, 0);
    archive_entry_set_birthtime(entry, entryts, 0);

    switch (type) {
    case G_FILE_TYPE_REGULAR:
    case G_FILE_TYPE_SYMBOLIC_LINK:
        if (g_file_info_get_is_backup(info) ||
            g_file_info_get_is_hidden(info)) {
            ret = 0;
            goto cleanup;
        }
        if (!g_str_has_suffix(entpath, ".rng") &&
            !g_str_has_suffix(entpath, ".xml") &&
            !g_str_has_suffix(entpath, ".ids")) {
            ret = 0;
            goto cleanup;
        }

        if (verbose) {
            g_print("%s: r %s\n", argv0, entpath);
        }
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_entry_set_size(entry, g_file_info_get_size(info));
        break;

    case G_FILE_TYPE_DIRECTORY:
        if (verbose) {
            g_print("%s: d %s\n", argv0, entpath);
        }

        archive_entry_set_filetype(entry, AE_IFDIR);
        archive_entry_set_perm(entry, 0755);
        archive_entry_set_size(entry, 0);
        break;

    case G_FILE_TYPE_SPECIAL:
        g_printerr("%s: cannot archive special file type %s\n",
                   argv0, abspath);
        goto cleanup;

    case G_FILE_TYPE_SHORTCUT:
        g_printerr("%s: cannot archive shortcut file type %s\n",
                   argv0, abspath);
        goto cleanup;

    case G_FILE_TYPE_MOUNTABLE:
        g_printerr("%s: cannot archive mount file type %s\n",
                   argv0, abspath);
        goto cleanup;

    case G_FILE_TYPE_UNKNOWN:
    default:
        g_printerr("%s: cannot archive unknown file type %s\n",
                   argv0, abspath);
        goto cleanup;
    }

    if (archive_write_header(arc, entry) != ARCHIVE_OK) {
        g_printerr("%s: cannot write archive header %s: %s\n",
                   argv0, target, archive_error_string(arc));
        goto cleanup;
    }

    switch (type) {
    case G_FILE_TYPE_REGULAR:
    case G_FILE_TYPE_SYMBOLIC_LINK:
        if (osinfo_db_export_create_reg(file, abspath, target, arc) < 0)
            goto cleanup;
        break;

    case G_FILE_TYPE_DIRECTORY:
        if (osinfo_db_export_create_dir(prefix, file, base, abspath, target, arc, verbose) < 0)
            goto cleanup;
        break;

    default:
        g_assert_not_reached();
    }

    ret = 0;
 cleanup:
    archive_entry_free(entry);
    g_free(abspath);
    g_free(relpath);
    g_free(entpath);
    g_object_unref(info);
    if (err)
        g_error_free(err);
    return ret;
}

static int osinfo_db_export_create_version(const gchar *prefix,
                                           const gchar *version,
                                           const gchar *target,
                                           struct archive *arc,
                                           gboolean verbose)
{
    int ret = -1;
    struct archive_entry *entry = NULL;
    gchar *entpath = NULL;

    entpath = g_strdup_printf("%s/VERSION", prefix);
    entry = archive_entry_new();
    archive_entry_set_pathname(entry, entpath);

    archive_entry_set_atime(entry, entryts, 0);
    archive_entry_set_ctime(entry, entryts, 0);
    archive_entry_set_mtime(entry, entryts, 0);
    archive_entry_set_birthtime(entry, entryts, 0);

    if (verbose) {
        g_print("%s: r %s\n", argv0, entpath);
    }
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_entry_set_size(entry, strlen(version));

    if (archive_write_header(arc, entry) != ARCHIVE_OK) {
        g_printerr("%s: cannot write archive header %s: %s\n",
                   argv0, target, archive_error_string(arc));
        goto cleanup;
    }

    if (archive_write_data(arc, version, strlen(version)) < 0) {
        g_printerr("%s: cannot write archive data for %s to %s: %s\n",
                   argv0, entpath, target, archive_error_string(arc));
        goto cleanup;
    }

    ret = 0;
 cleanup:
    archive_entry_free(entry);
    return ret;
}

static int osinfo_db_export_create_license(const gchar *prefix,
                                           const gchar *license,
                                           const gchar *target,
                                           struct archive *arc,
                                           gboolean verbose)
{
    int ret = -1;
    struct archive_entry *entry = NULL;
    gchar *entpath = NULL;
    GFile *file = NULL;
    GFileInfo *info = NULL;
    GError *err = NULL;

    file = g_file_new_for_path(license);

    info = g_file_query_info(file,
                             G_FILE_ATTRIBUTE_STANDARD_NAME
                             ","
                             G_FILE_ATTRIBUTE_STANDARD_SIZE,
                             G_FILE_QUERY_INFO_NONE,
                             NULL,
                             &err);
    if (!info) {
        g_printerr("%s: cannot get file info %s: %s\n",
                   argv0, license, err->message);
        goto cleanup;
    }

    entpath = g_strdup_printf("%s/LICENSE", prefix);
    entry = archive_entry_new();
    archive_entry_set_pathname(entry, entpath);

    archive_entry_set_atime(entry, entryts, 0);
    archive_entry_set_ctime(entry, entryts, 0);
    archive_entry_set_mtime(entry, entryts, 0);
    archive_entry_set_birthtime(entry, entryts, 0);

    if (verbose) {
        g_print("%s: r %s\n", argv0, entpath);
    }
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_entry_set_size(entry, g_file_info_get_size(info));

    if (archive_write_header(arc, entry) != ARCHIVE_OK) {
        g_printerr("%s: cannot write archive header %s: %s\n",
                   argv0, target, archive_error_string(arc));
        goto cleanup;
    }

    if (osinfo_db_export_create_reg(file, license, target, arc) < 0)
        goto cleanup;

    ret = 0;
 cleanup:
    archive_entry_free(entry);
    if (err)
        g_error_free(err);
    return ret;
}

static int osinfo_db_export_create(const gchar *prefix,
                                   const gchar *version,
                                   GFile *source,
                                   const gchar *target,
                                   const gchar *license,
                                   gboolean verbose)
{
    struct archive *arc;
    int ret = -1;
    int r;
    GError *err = NULL;

    arc = archive_write_new();

    archive_write_add_filter_xz(arc);
    archive_write_set_format_pax(arc);

    if (target != NULL && g_str_equal(target, "-"))
        target = NULL;

    if ((r = archive_write_open_filename(arc, target)) != ARCHIVE_OK) {
        g_printerr("%s: cannot open archive %s: %s\n",
                   argv0, target, archive_error_string(arc));
        goto cleanup;
    }

    if (osinfo_db_export_create_file(prefix, source, NULL, source, target, arc, verbose) < 0) {
        goto cleanup;
    }

    if (osinfo_db_export_create_version(prefix, version, target, arc, verbose) < 0) {
        goto cleanup;
    }

    if (license != NULL &&
        osinfo_db_export_create_license(prefix, license, target, arc, verbose) < 0) {
        goto cleanup;
    }

    if (archive_write_close(arc) != ARCHIVE_OK) {
        g_printerr("%s: cannot finish writing archive %s: %s\n",
                   argv0, target, archive_error_string(arc));
        goto cleanup;
    }

    ret = 0;
 cleanup:
    archive_write_free(arc);
    if (err)
        g_error_free(err);
    return ret;
}


static gchar *osinfo_db_version(void)
{
    GTimeZone *tz = g_time_zone_new_utc();
    GDateTime *now = g_date_time_new_now(tz);
    gchar *ret;

    ret = g_strdup_printf("%04d%02d%02d",
                          g_date_time_get_year(now),
                          g_date_time_get_month(now),
                          g_date_time_get_day_of_month(now));
    g_date_time_unref(now);
    g_time_zone_unref(tz);
    return ret;
}


gint main(gint argc, gchar **argv)
{
    GOptionContext *context;
    GError *error = NULL;
    gint ret = EXIT_FAILURE;
    gboolean verbose = FALSE;
    gboolean user = FALSE;
    gboolean local = FALSE;
    gboolean system = FALSE;
    const gchar *root = "";
    gchar *archive = NULL;
    const gchar *custom = NULL;
    const gchar *version = NULL;
    gchar *autoversion = NULL;
    gchar *prefix = NULL;
    const gchar *license = NULL;
    int locs = 0;
    GFile *dir = NULL;
    const GOptionEntry entries[] = {
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, (void*)&verbose,
        N_("Verbose progress information"), NULL, },
      { "user", 0, 0, G_OPTION_ARG_NONE, (void *)&user,
        N_("Install into user directory"), NULL, },
      { "local", 0, 0, G_OPTION_ARG_NONE, (void *)&local,
        N_("Install into local directory"), NULL, },
      { "system", 0, 0, G_OPTION_ARG_NONE, (void *)&system,
        N_("Install into system directory"), NULL, },
      { "dir", 0, 0, G_OPTION_ARG_STRING, (void *)&custom,
        N_("Import into custom directory"), NULL, },
      { "version", 0, 0, G_OPTION_ARG_STRING, (void *)&version,
        N_("Set version number of archive"), NULL, },
      { "root", 0, 0, G_OPTION_ARG_STRING, &root,
        N_("Installation root directory"), NULL, },
      { "license", 0, 0, G_OPTION_ARG_STRING, &license,
        N_("License file"), NULL, },
      { NULL, 0, 0, 0, NULL, NULL, NULL },
    };
    argv0 = argv[0];

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    context = g_option_context_new(_("- Export database archive "));

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("%s: error while parsing commandline options: %s\n\n"),
                   argv0, error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));
        goto error;
    }

    if (argc > 2) {
        g_printerr(_("%s: expected path to one archive file to export\n"),
                   argv0);
        goto error;
    }

    if (local)
        locs++;
    if (system)
        locs++;
    if (user)
        locs++;
    if (custom)
        locs++;
    if (locs > 1) {
        g_printerr(_("Only one of --user, --local, --system & --dir can be used"));
        goto error;
    }

    entryts = time(NULL);
    if (version == NULL) {
        autoversion = osinfo_db_version();
        version = autoversion;
    }
    prefix = g_strdup_printf("osinfo-db-%s", version);
    if (argc == 2) {
        archive = g_strdup(argv[1]);
    } else {
        archive = g_strdup_printf("%s.tar.xz", prefix);
    }
    dir = osinfo_db_get_path(root, user, local, system, custom);
    if (osinfo_db_export_create(prefix, version, dir, archive,
                                license, verbose) < 0)
        goto error;

    ret = EXIT_SUCCESS;

 error:
    g_free(archive);
    g_free(autoversion);
    g_free(prefix);
    if (dir) {
        g_object_unref(dir);
    }
    g_clear_error(&error);
    g_option_context_free(context);

    return ret;
}


/*
=pod

=head1 NAME

osinfo-db-export - Export to a osinfo database archive

=head1 SYNOPSIS

osinfo-db-export [OPTIONS...] [ARCHIVE-FILE]

=head1 DESCRIPTION

The B<osinfo-db-export> tool will create an osinfo database
archive file containing the content from one of the standard
local database locations:

=over 1

=item B<system>

This is the primary system-wide database location, intended
for use by operating system vendors distributing database
files in the native package format.

=item B<local>

This is the secondary system-wide database location, intended
for use by system administrators wishing to provide an updated
database for all users.

=item B<user>

This is the user private database location, intended for use
by unprivileged local users wishing to provide applications
they use with an updated database.

=back

If run by a privileged account (ie root), the B<local> database
location will be used by default, otherwise the B<user> location
will be used.

If no B<ARCHIVE-FILE> path is given, an automatically generated
filename will be used, taking the format B<osinfo-db-$VERSION.tar.xz>.

=head1 OPTIONS

=over 8

=item B<--user>

Override the default behaviour to force archiving files from the
B<user> database location.

=item B<--local>

Override the default behaviour to force archiving files from the
B<local> database location.

=item B<--system>

Override the default behaviour to force archiving files from the
B<system> database location.

=item B<--dir=PATH>

Override the default behaviour to force archiving files from the
custom directory B<PATH>.

=item B<--root=PATH>

Prefix the database location with the root directory given by
C<PATH>. This is useful when wishing to archive files that are
in a chroot environment or equivalent.

=item B<--version=VERSION>

Set the version string for the files in the archive to
B<VERSION>. If this argument is not given, the version
will be set to the current date in the format B<YYYYMMDD>.

=item B<--license=LICENSE-FILE>

Add C<LICENSE-FILE> to the generated archive as an entry
named "LICENSE".

=item B<-v>, B<--verbose>

Display verbose progress information when archiving files

=back

=head1 EXIT STATUS

The exit status will be 0 if all files were packed
successfully, or 1 if at least one file could not be
packed into the archive.

=head1 SEE ALSO

C<osinfo-db-import(1)>, C<osinfo-db-path(1)>

=head1 AUTHORS

Daniel P. Berrange <berrange@redhat.com>

=head1 COPYRIGHT

Copyright (C) 2016 Red Hat, Inc.

=head1 LICENSE

C<osinfo-db-export> is distributed under the terms of the GNU LGPL v2+
license. This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE

=cut
*/

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
