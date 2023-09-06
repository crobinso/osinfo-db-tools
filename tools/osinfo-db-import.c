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
 * with this program. If not, see <http://www.gnu.org/licenses/>
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#include <locale.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <stdlib.h>
#include <archive.h>
#include <archive_entry.h>
#include <libsoup/soup.h>

#include "osinfo-db-util.h"

#define LATEST_URI "https://db.libosinfo.org/latest.json"
#define NIGHTLY_URI "https://db.libosinfo.org/nightly.json"
#define VERSION_FILE "VERSION"

#if SOUP_MAJOR_VERSION < 3
# define soup_message_get_status(message) message->status_code
# define soup_message_get_response_headers(message) message->response_headers
#endif

const char *argv0;
static SoupSession *session = NULL;

static int osinfo_db_import_create_reg(GFile *file,
                                       struct archive *arc,
                                       struct archive_entry *entry)
{
    g_autoptr(GFileOutputStream) os = NULL;
    g_autoptr(GError) err = NULL;
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
            return -1;
        }

        if (!g_seekable_seek(G_SEEKABLE(os), offset, G_SEEK_SET, NULL, NULL)) {
            g_printerr("%s: cannot seek to %" G_GUINT64_FORMAT " in %s\n",
                       argv0, (uint64_t)offset, g_file_get_path(file));
            return -1;
        }
        if (!g_output_stream_write_all(G_OUTPUT_STREAM(os), buf, size, NULL, NULL, NULL)) {
            g_printerr("%s: cannot write to %s\n",
                       argv0, g_file_get_path(file));
            return -1;
        }
    }
    return 0;
}

static int osinfo_db_import_create_dir(GFile *file,
                                       struct archive_entry *entry)
{
    g_autoptr(GError) err = NULL;
    if (!g_file_make_directory_with_parents(file, NULL, &err) &&
        err->code != G_IO_ERROR_EXISTS) {
        g_printerr("%s: %s\n", argv0, err->message);
        return -1;
    }
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

static gchar *
osinfo_db_import_download_file(const gchar *source)
{
    g_autoptr(GInputStream) stream = NULL;
    g_autoptr(GFile) out = NULL;
    g_autoptr(GError) err = NULL;
    g_autoptr(SoupMessage) message = NULL;
    g_autofree gchar *filename = NULL;
    g_autofree gchar *content = NULL;
    gchar *source_file = NULL;
    goffset content_size = 0;
    GFileCreateFlags flags = G_FILE_CREATE_REPLACE_DESTINATION;
    int ret = -1;

    filename = g_path_get_basename(source);
    if (filename == NULL)
        goto cleanup;

    source_file = g_build_filename(g_get_user_cache_dir(), filename, NULL);
    if (source_file == NULL)
        goto cleanup;

    out = g_file_new_for_path(source_file);
    if (out == NULL)
        goto cleanup;

    if (session == NULL)
        session = soup_session_new();

    if (session == NULL)
        goto cleanup;

    message = soup_message_new("GET", source);
    if (message == NULL)
        goto cleanup;

    stream = soup_session_send(session, message, NULL, &err);
    if (stream == NULL ||
        !SOUP_STATUS_IS_SUCCESSFUL(soup_message_get_status(message))) {
        g_printerr("Could not access %s: %s\n",
                   source,
                   err != NULL ? err->message :
                                 soup_status_get_phrase(soup_message_get_status(message)));
        goto cleanup;
    }

    content_size = soup_message_headers_get_content_length(soup_message_get_response_headers(message));
    content = g_malloc0(content_size);

    if (!g_input_stream_read_all(stream, content, content_size, NULL, NULL, &err)) {
        g_printerr("Could not load the content of %s: %s\n",
                   source, err->message);
        goto cleanup;
    }

    if (!g_file_replace_contents(out, content, content_size, NULL, TRUE, flags, NULL, NULL, &err)) {
        g_printerr("Could not download file \"%s\": %s\n",
                   source, err->message);
        goto cleanup;
    }

    ret = 0;

 cleanup:
    if (ret != 0 && source_file != NULL) {
        unlink(source_file);
        g_free(source_file);
        source_file = NULL;
    }

    return source_file;
}

static gboolean osinfo_db_get_installed_version(GFile *dir,
                                                gchar **version)
{
    g_autoptr(GFile) file = NULL;
    g_autoptr(GError) err = NULL;

    file = g_file_get_child(dir, VERSION_FILE);
    if (file == NULL)
        return FALSE;

    if (!g_file_load_contents(file, NULL, version, NULL, NULL, &err)) {
        if (!g_error_matches(err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
            g_printerr("Failed get the content of file %s: %s\n",
                       VERSION_FILE, err->message);
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean osinfo_db_get_info(const gchar *from_url,
                                   gchar **version,
                                   gchar **url)
{
    g_autoptr(SoupMessage) message = NULL;
    g_autoptr(GInputStream) stream = NULL;
    g_autoptr(JsonParser) parser = NULL;
    g_autoptr(JsonReader) reader = NULL;
    g_autoptr(GError) err = NULL;
    g_autofree gchar *content = NULL;
    goffset content_size = 0;

    if (session == NULL)
        session = soup_session_new();

    if (session == NULL)
        return FALSE;

    message = soup_message_new("GET", from_url);
    if (message == NULL)
        return FALSE;

    stream = soup_session_send(session, message, NULL, &err);
    if (stream == NULL ||
        !SOUP_STATUS_IS_SUCCESSFUL(soup_message_get_status(message))) {
        g_printerr("Could not access %s: %s\n",
                   from_url,
                   err != NULL ? err->message :
                                 soup_status_get_phrase(soup_message_get_status(message)));
        return FALSE;
    }

    content_size = soup_message_headers_get_content_length(soup_message_get_response_headers(message));
    content = g_malloc0(content_size);

    if (!g_input_stream_read_all(stream, content, content_size, NULL, NULL, &err)) {
        g_printerr("Could not load the content of %s: %s\n",
                   from_url, err->message);
        return FALSE;
    }

    parser = json_parser_new();
    if (parser == NULL) {
        g_printerr("Failed to create the json parser\n");
        return FALSE;
    }

    if (!json_parser_load_from_data(parser, content, -1, &err)) {
        g_printerr("Failed to parse the content of %s: %s\n",
                   from_url, err->message);
        return FALSE;
    }

    reader = json_reader_new(json_parser_get_root(parser));
    if (reader == NULL) {
        g_printerr("Failed to create the reader for the json parser\n");
        return FALSE;
    }

    if (!json_reader_read_member(reader, "release")) {
        const GError *error = json_reader_get_error(reader);
        g_printerr("Failed to read the \"release\" member of the %s file: %s\n",
                   from_url, error->message);
        return FALSE;
    }

    if (version && !json_reader_read_member(reader, "version")) {
        const GError *error = json_reader_get_error(reader);
        g_printerr("Failed to read the \"version\" member of the %s file: %s\n",
                   from_url, error->message);
        return FALSE;
    }

    if (version) {
        *version = g_strdup(json_reader_get_string_value(reader));
        if (*version == NULL)
            return FALSE;

        json_reader_end_member(reader); /* "version" */
    }

    if (!json_reader_read_member(reader, "archive")) {
        const GError *error = json_reader_get_error(reader);
        g_printerr("Failed to read the \"archive\" member of the %s file: %s\n",
                   from_url, error->message);
        return FALSE;
    }

    *url = g_strdup(json_reader_get_string_value(reader));
    if (*url == NULL)
        return FALSE;

    json_reader_end_member(reader); /* "archive" */
    json_reader_end_member(reader); /* "release" */

    return TRUE;
}

static gboolean osinfo_db_get_latest_info(gchar **version,
                                          gchar **url)
{
    return osinfo_db_get_info(LATEST_URI, version, url);
}

static gboolean osinfo_db_get_nightly_info(gchar **url)
{
    return osinfo_db_get_info(NIGHTLY_URI, NULL, url);
}

static gboolean requires_soup(const gchar *source)
{
    const gchar *prefixes[] = { "http://", "https://", NULL };
    gsize i;

    for (i = 0; prefixes[i] != NULL; i++) {
        if (g_str_has_prefix(source, prefixes[i]))
            return TRUE;
    }

    return FALSE;
}

static int osinfo_db_import_extract(GFile *target,
                                    const char *source,
                                    gboolean verbose)
{
    struct archive *arc;
    struct archive_entry *entry;
    int ret = -1;
    int r;
    g_autoptr(GFile) file = NULL;
    g_autofree gchar *source_file = NULL;
    gboolean file_is_native = TRUE;

    arc = archive_read_new();

    archive_read_support_format_tar(arc);
    archive_read_support_filter_xz(arc);

    if (source != NULL && g_str_equal(source, "-"))
        source = NULL;

    if (source != NULL) {
        file_is_native = !requires_soup(source);

        if (file_is_native) {
            file = g_file_new_for_commandline_arg(source);
            if (file == NULL)
                goto cleanup;

            source_file = g_file_get_path(file);
            g_clear_object(&file);
        } else {
            source_file = osinfo_db_import_download_file(source);
        }

        if (source_file == NULL)
            goto cleanup;
    }

    if ((r = archive_read_open_filename(arc, source_file, 10240)) != ARCHIVE_OK) {
        g_printerr("%s: cannot open archive %s: %s\n",
                   argv0, source_file, archive_error_string(arc));
        goto cleanup;
    }

    for (;;) {
        r = archive_read_next_header(arc, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK) {
            g_printerr("%s: cannot read next archive entry in %s: %s\n",
                       argv0, source_file, archive_error_string(arc));
            goto cleanup;
        }

        file = osinfo_db_import_get_file(target, entry);
        if (osinfo_db_import_create(file, arc, entry, verbose) < 0) {
            goto cleanup;
        }
        g_clear_object(&file);
    }

    if (archive_read_close(arc) != ARCHIVE_OK) {
        g_printerr("%s: cannot finish reading archive %s: %s\n",
                   argv0, source_file, archive_error_string(arc));
        goto cleanup;
    }

    ret = 0;
 cleanup:
    archive_read_free(arc);
    if (!file_is_native && source_file != NULL)
        unlink(source_file);
    return ret;
}

gint main(gint argc, gchar **argv)
{
    g_autoptr(GOptionContext) context = NULL;
    g_autoptr(GError) error = NULL;
    g_autoptr(GFile) dir = NULL;
    gboolean verbose = FALSE;
    gboolean user = FALSE;
    gboolean local = FALSE;
    gboolean system = FALSE;
    gboolean latest = FALSE;
    gboolean nightly = FALSE;
    g_autofree gchar *installed_version = NULL;
    g_autofree gchar *latest_version = NULL;
    g_autofree gchar *archive_url = NULL;
    const gchar *root = "";
    const gchar *archive = NULL;
    const gchar *custom = NULL;
    int locs = 0;
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
      { "latest", 0, 0, G_OPTION_ARG_NONE, (void *)&latest,
        N_("Import the latest osinfo-db from osinfo-db's website"), NULL, },
      { "nightly", 0, 0, G_OPTION_ARG_NONE, (void *)&nightly,
        N_("Import the latest nightly build of unreleased osinfo-db from osinfo-db's website"), NULL, },
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
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        g_printerr(_("%s: expected path to one archive file to import\n"),
                   argv0);
        return EXIT_FAILURE;
    }

    /* check mutual exclusivity of --latest and --nightly */
    if (latest && nightly)
    {
         g_printerr(_("--latest and --nightly are mutually exclusive\n"));
         return EXIT_FAILURE;
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
        g_printerr(_("Only one of --user, --local, --system & --dir can be used\n"));
        return EXIT_FAILURE;
    }

    archive = argc == 2 ? argv[1] : NULL;
    dir = osinfo_db_get_path(root, user, local, system, custom);

    if (nightly) {
        if (!osinfo_db_get_nightly_info(&archive_url))
            return EXIT_FAILURE;

        archive = archive_url;
    } else if (latest) {
        if (!osinfo_db_get_installed_version(dir, &installed_version))
            return EXIT_FAILURE;

        if (!osinfo_db_get_latest_info(&latest_version, &archive_url))
            return EXIT_FAILURE;

        if (g_strcmp0(latest_version, installed_version) <= 0) {
            return EXIT_SUCCESS;
        }

        archive = archive_url;
    }

    if (osinfo_db_import_extract(dir, archive, verbose) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}


/*
=pod

=head1 NAME

osinfo-db-import - Import an osinfo database archive

=head1 SYNOPSIS

osinfo-db-import [OPTIONS...] [ARCHIVE-FILE]

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

When passing a non local ARCHIVE-FILE, only http:// and https://
protocols are supported.

With no ARCHIVE-FILE, or when ARCHIVE-FILE is -, read standard
input.

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

=item B<--latest>

Downloads the latest osinfo-db release from libosinfo's official
releases website and installs it in the desired location.
The latest osinfo-db release is only downloaded and installed
when it's newer than the one installed in the desired location.
Note that this option is mutually exclusive with '--nightly'.

=item B<--nightly>

Downloads the nightly (unreleased) osinfo-db build from libosinfo's
website and installs it in the desired location.
Unlike with '--latest' with this option the nightly archive always
downloaded and installed regardless of the version installed in the
desired location. Note that this option is mutually exclusive with
'--latest'.

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
