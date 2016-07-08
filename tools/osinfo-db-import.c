/*
 * Copyright (C) 2016 Red Hat, Inc
 *
 * osinfo-db-import: import a database archive
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

static int osinfo_db_import_create_reg(GFile *file,
                                       struct archive *arc,
                                       struct archive_entry *entry)
{
    GFileOutputStream *os = NULL;
    GError *err = NULL;
    int ret;
    int r;
    const void *buf;
    size_t size;
    gint64 offset;

    os = g_file_replace(file, NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION,
                        NULL, &err);
    if (!os) {
        g_printerr("%s: %s\n",
                   argv0,  err->message);
        return -1;
    }

    for (;;) {
        r = archive_read_data_block(arc, &buf, &size, &offset);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK) {
            g_printerr("%s: cannot write to %s\n",
                       argv0, g_file_get_path(file));
            goto cleanup;
        }

        if (!g_seekable_seek(G_SEEKABLE(os), offset, G_SEEK_SET, NULL, NULL)) {
            g_printerr("%s: cannot seek to %" PRId64 " in %s\n",
                       argv0, (uint64_t)offset, g_file_get_path(file));
            goto cleanup;
        }
        if (!g_output_stream_write_all(G_OUTPUT_STREAM(os), buf, size, NULL, NULL, NULL)) {
            g_printerr("%s: cannot write to %s\n",
                       argv0, g_file_get_path(file));
            goto cleanup;
        }
    }
    ret = 0;
 cleanup:
    g_object_unref(os);
    return ret;
}

static int osinfo_db_import_create_dir(GFile *file,
                                       struct archive_entry *entry)
{
    GError *err = NULL;
    if (!g_file_make_directory_with_parents(file, NULL, &err) &&
        err->code != G_IO_ERROR_EXISTS) {
        g_printerr("%s: %s\n", argv0, err->message);
        return -1;
    }
    if (err)
        g_error_free(err);
    return 0;
}


static int osinfo_db_import_create(GFile *file,
                                   struct archive *arc,
                                   struct archive_entry *entry,
                                   gboolean verbose)
{
    int type = archive_entry_filetype(entry) & AE_IFMT;

    switch (type) {
    case AE_IFREG:
        if (verbose) {
            g_print("%s: r %s\n", argv0, archive_entry_pathname(entry));
        }
        return osinfo_db_import_create_reg(file, arc, entry);

    case AE_IFDIR:
        if (verbose) {
            g_print("%s: d %s\n", argv0, archive_entry_pathname(entry));
        }
        return osinfo_db_import_create_dir(file, entry);

    default:
        g_printerr("%s: unsupported file type for %s\n",
                   argv0, archive_entry_pathname(entry));
        return -1;
    }
}

static GFile *osinfo_db_import_get_file(GFile *target,
                                        struct archive_entry *entry)
{
    const gchar *entpath = archive_entry_pathname(entry);
    const gchar *tmp = strchr(entpath, '/');
    if (!tmp) {
        tmp = "";
    } else {
        tmp++;
    }
    return g_file_resolve_relative_path(target, tmp);
}

static int osinfo_db_import_extract(GFile *target,
                                    const char *source,
                                    gboolean verbose)
{
    struct archive *arc;
    struct archive_entry *entry;
    int ret = -1;
    int r;
    GFile *file = NULL;
    GError *err = NULL;

    arc = archive_read_new();

    archive_read_support_format_tar(arc);
    archive_read_support_filter_xz(arc);

    if (source != NULL && g_str_equal(source, "-"))
        source = NULL;

    if ((r = archive_read_open_filename(arc, source, 10240)) != ARCHIVE_OK) {
        g_printerr("%s: cannot open archive %s: %s\n",
                   argv0, source, archive_error_string(arc));
        goto cleanup;
    }

    for (;;) {
        r = archive_read_next_header(arc, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK) {
            g_printerr("%s: cannot read next archive entry in %s: %s\n",
                       argv0, source, archive_error_string(arc));
            goto cleanup;
        }

        file = osinfo_db_import_get_file(target, entry);
        if (osinfo_db_import_create(file, arc, entry, verbose) < 0) {
            goto cleanup;
        }
        g_object_unref(file);
        file = NULL;
    }

    if (archive_read_close(arc) != ARCHIVE_OK) {
        g_printerr("%s: cannot finish reading archive %s: %s\n",
                   argv0, source, archive_error_string(arc));
        goto cleanup;
    }

    ret = 0;
 cleanup:
    archive_read_free(arc);
    if (err)
        g_error_free(err);
    if (file)
        g_object_unref(file);
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
    const gchar *archive = NULL;
    const gchar *custom = NULL;
    int locs = 0;
    GFile *dir = NULL;
    const GOptionEntry entries[] = {
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, (void*)&verbose,
        N_("Verbose progress information"), NULL, },
      { "user", 0, 0, G_OPTION_ARG_NONE, (void *)&user,
        N_("Import into user directory"), NULL, },
      { "local", 0, 0, G_OPTION_ARG_NONE, (void *)&local,
        N_("Import into local directory"), NULL, },
      { "system", 0, 0, G_OPTION_ARG_NONE, (void *)&system,
        N_("Import into system directory"), NULL, },
      { "dir", 0, 0, G_OPTION_ARG_STRING, (void *)&custom,
        N_("Import into custom directory"), NULL, },
      { "root", 0, 0, G_OPTION_ARG_STRING, &root,
        N_("Installation root directory"), NULL, },
      { NULL, 0, 0, 0, NULL, NULL, NULL },
    };
    argv0 = argv[0];

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    context = g_option_context_new(_("- Import database archive "));

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("%s: error while parsing commandline options: %s\n\n"),
                   argv0, error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));
        goto error;
    }

    if (argc > 2) {
        g_printerr(_("%s: expected path to one archive file to import\n"),
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

    archive = argc == 2 ? argv[1] : NULL;
    dir = osinfo_db_get_path(root, user, local, system, custom);
    if (osinfo_db_import_extract(dir, archive, verbose) < 0)
        goto error;

    ret = EXIT_SUCCESS;

 error:
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

osinfo-db-import - Import an osinfo database archive

=head1 SYNOPSIS

osinfo-db-import [OPTIONS...] ARCHIVE-FILE

=head1 DESCRIPTION

The B<osinfo-db-import> tool will take an osinfo database
archive and extract its contents into one of the standard
database locations on the current host:

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

=head1 OPTIONS

=over 8

=item B<--user>

Override the default behaviour to force installation into the
B<user> database location.

=item B<--local>

Override the default behaviour to force installation into the
B<local> database location.

=item B<--system>

Override the default behaviour to force installation into the
B<system> database location.

=item B<--dir=PATH>

Override the default behaviour to force installation into the
custom directory B<PATH>.

=item B<--root=PATH>

Prefix the installation location with the root directory
given by C<PATH>. This is useful when wishing to install
into a chroot environment or equivalent.

=item B<-v>, B<--verbose>

Display verbose progress information when installing files

=back

=head1 EXIT STATUS

The exit status will be 0 if all files were installed
successfully, or 1 if at least one file could not be
installed.

=head1 SEE ALSO

C<osinfo-db-export(1)>, C<osinfo-db-path(1)>

=head1 AUTHORS

Daniel P. Berrange <berrange@redhat.com>

=head1 COPYRIGHT

Copyright (C) 2016 Red Hat, Inc.

=head1 LICENSE

C<osinfo-db-import> is distributed under the terms of the GNU LGPL v2+
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
