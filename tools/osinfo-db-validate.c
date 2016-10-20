/*
 * Copyright (C) 2012-2016 Red Hat, Inc
 *
 * osinfo-validate: validate that XML file(s) follows the published schema
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

#include <libxml/relaxng.h>
#include <locale.h>
#include <glib/gi18n.h>

#include "osinfo-db-util.h"

static gboolean verbose = FALSE;

static void validate_generic_error_nop(void *userData G_GNUC_UNUSED,
                                       const char *msg G_GNUC_UNUSED,
                                       ...)
{
}

static void validate_structured_error_nop(void *userData G_GNUC_UNUSED,
                                          xmlErrorPtr error G_GNUC_UNUSED)
{
    if (error->file)
        g_printerr("%s:%d %s", error->file, error->line, error->message);
    else
        g_printerr(_("Schema validity error %s"), error->message);
}

static xmlDocPtr parse_file(GFile *file, GError **error)
{
    xmlDocPtr doc = NULL;
    xmlParserCtxtPtr pctxt;
    gchar *data = NULL;
    gsize length;
    gchar *uri = g_file_get_uri(file);

    if (!g_file_load_contents(file, NULL, &data, &length, NULL, error))
        goto cleanup;

    if (!(pctxt = xmlNewParserCtxt())) {
        g_set_error(error, OSINFO_DB_ERROR, 0, "%s",
                    _("Unable to create libxml parser"));
        goto cleanup;
    }

    if (!(doc = xmlCtxtReadDoc(pctxt, (const xmlChar*)data, uri, NULL,
                               XML_PARSE_NOENT | XML_PARSE_NONET |
                               XML_PARSE_NOWARNING))) {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    _("Unable to parse XML document '%s'"),
                    uri);
        goto cleanup;
    }

 cleanup:
    g_free(uri);
    g_free(data);
    return doc;
}

static gboolean validate_file(xmlRelaxNGValidCtxtPtr rngValid, GFile *file, GFileInfo *info, GError **error);


static gboolean validate_file_regular(xmlRelaxNGValidCtxtPtr rngValid,
                                      GFile *file,
                                      GError **error)
{
    gboolean ret = FALSE;
    xmlDocPtr doc = NULL;
    gchar *uri = g_file_get_uri(file);

    if (!g_str_has_suffix(uri, ".xml")) {
        ret = TRUE;
        goto cleanup;
    }

    if (!(doc = parse_file(file, error)))
        goto cleanup;

    if (xmlRelaxNGValidateDoc(rngValid, doc) != 0) {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    _("Unable to validate XML document '%s'"),
                    uri);
        goto cleanup;
    }

    ret = TRUE;

 cleanup:
    //g_free(uri);
    xmlFreeDoc(doc);
    return ret;
}


static gboolean validate_file_directory(xmlRelaxNGValidCtxtPtr rngValid, GFile *file, GError **error)
{
    gboolean ret = FALSE;
    GFileEnumerator *children = NULL;
    GFileInfo *info = NULL;

    if (!(children = g_file_enumerate_children(file, "standard::*", 0, NULL, error)))
        goto cleanup;

    while ((info = g_file_enumerator_next_file(children, NULL, error))) {
        GFile *child = g_file_get_child(file, g_file_info_get_name(info));
        if (!validate_file(rngValid, child, info, error)) {
            g_object_unref(child);
            goto cleanup;
        }
        g_object_unref(child);
    }

    if (*error)
        goto cleanup;

    ret = TRUE;

 cleanup:
    g_object_unref(children);
    return ret;
}


static gboolean validate_file(xmlRelaxNGValidCtxtPtr rngValid, GFile *file, GFileInfo *info, GError **error)
{
    gboolean ret = FALSE;
    GFileInfo *thisinfo = NULL;
    gchar *uri = g_file_get_uri(file);

    if (verbose)
        g_print(_("Processing '%s'...\n"), uri);

    if (!info) {
        if (!(thisinfo = g_file_query_info(file, "standard::*",
                                           G_FILE_QUERY_INFO_NONE,
                                           NULL, error)))
            goto cleanup;
        info = thisinfo;
    }

    if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
        if (!validate_file_directory(rngValid, file, error))
            goto cleanup;
    } else if (g_file_info_get_file_type(info) == G_FILE_TYPE_REGULAR) {
        if (!validate_file_regular(rngValid, file, error))
            goto cleanup;
    } else {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    "Unable to handle file type for %s",
                    uri);
        goto cleanup;
    }

    ret = TRUE;

 cleanup:
    g_free(uri);
    if (thisinfo)
        g_object_unref(thisinfo);
    return ret;
}


static gboolean validate_files(GFile *schema, gsize nfiles, GFile **files, GError **error)
{
    xmlRelaxNGParserCtxtPtr rngParser = NULL;
    xmlRelaxNGPtr rng = NULL;
    xmlRelaxNGValidCtxtPtr rngValid = NULL;
    gboolean ret = FALSE;
    gsize i;
    gchar *schemapath = NULL;

    xmlSetGenericErrorFunc(NULL, validate_generic_error_nop);
    xmlSetStructuredErrorFunc(NULL, validate_structured_error_nop);

    schemapath = g_file_get_path(schema);
    rngParser = xmlRelaxNGNewParserCtxt(schemapath);
    if (!rngParser) {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    _("Unable to create RNG parser for %s"),
                    schemapath);
        goto cleanup;
    }

    rng = xmlRelaxNGParse(rngParser);
    if (!rng) {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    _("Unable to parse RNG %s"),
                    schemapath);
        goto cleanup;
    }

    rngValid = xmlRelaxNGNewValidCtxt(rng);
    if (!rngValid) {
        g_set_error(error, OSINFO_DB_ERROR, 0,
                    _("Unable to create RNG validation context %s"),
                    schemapath);
        goto cleanup;
    }

    for (i = 0; i < nfiles; i++) {
        if (!validate_file(rngValid, files[i], NULL, error))
            goto cleanup;
    }

    ret = TRUE;

 cleanup:
    g_free(schemapath);
    xmlRelaxNGFreeValidCtxt(rngValid);
    xmlRelaxNGFreeParserCtxt(rngParser);
    xmlRelaxNGFree(rng);
    return ret;
}

gint main(gint argc, gchar **argv)
{
    GOptionContext *context;
    GError *error = NULL;
    gint ret = EXIT_FAILURE;
    GFile *schema = NULL;
    gboolean user = FALSE;
    gboolean local = FALSE;
    gboolean system = FALSE;
    const gchar *root = "";
    const gchar *custom = NULL;
    int locs = 0;
    GFile *dir = NULL;
    GFile **files = NULL;
    gsize nfiles = 0, i;
    const GOptionEntry entries[] = {
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, (void*)&verbose,
        N_("Verbose progress information"), NULL, },
      { "user", 0, 0, G_OPTION_ARG_NONE, (void *)&user,
        N_("Validate files in user directory"), NULL, },
      { "local", 0, 0, G_OPTION_ARG_NONE, (void *)&local,
        N_("Validate files in local directory"), NULL, },
      { "system", 0, 0, G_OPTION_ARG_NONE, (void *)&system,
        N_("Validate files in system directory"), NULL, },
      { "dir", 0, 0, G_OPTION_ARG_STRING, (void *)&custom,
        N_("Validate files in custom directory"), NULL, },
      { "root", 0, 0, G_OPTION_ARG_STRING, &root,
        N_("Installation root directory"), NULL, },
      { NULL, 0, 0, 0, NULL, NULL, NULL },
    };

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    context = g_option_context_new(_("- Validate XML documents "));

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("Error while parsing commandline options: %s\n"),
                   error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));
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
    if (locs > 1 || (locs && argc > 1)) {
        g_printerr(_("Only one of --user, --local, --system, --dir or positional filenames can be used"));
        goto error;
    }

    schema = osinfo_db_get_file(root,
                                user || custom,
                                local || user || custom,
                                system || local || user || custom,
                                custom,
                                "schema/osinfo.rng", &error);
    if (!schema) {
        g_printerr("%s\n", error->message);
        goto error;
    }

    dir = osinfo_db_get_path(root, user, local, system, custom);

    if (dir) {
        files = g_new0(GFile *, 1);
        files[nfiles++] = dir;
    } else {
        files = g_new0(GFile *, argc - 1);
        for (i = 1; i < argc; i++) {
            files[nfiles++] = g_file_new_for_commandline_arg(argv[i]);
        }
    }
    if (!validate_files(schema, nfiles, files, &error)) {
        g_printerr("%s\n", error->message);
        goto error;
    }

    ret = EXIT_SUCCESS;

 error:
    if (schema)
        g_object_unref(schema);
    if (dir)
        g_object_unref(dir);
    g_clear_error(&error);
    g_option_context_free(context);

    return ret;
}

/*
=pod

=head1 NAME

osinfo-db-validate - Validate libosinfo XML data files

=head1 SYNOPSIS

osinfo-db-validate [OPTIONS...]

osinfo-db-validate [OPTIONS...] LOCAL-PATH1 [LOCAL-PATH2...]

osinfo-db-validate [OPTIONS...] URI1 [URI2...]

=head1 DESCRIPTION

The B<osinfo-db-validate> tool is able to validate XML files
in one of the osinfo database locations for compliance with
the RNG schema.

=over 1

=item B<system>

This is the primary system-wide database location, intended
for use by operating system vendors distributing database
files in the native package format. The RNG schema is
expected to be present in this location.

=item B<local>

This is the secondary system-wide database location, intended
for use by system administrators wishing to provide an updated
database for all users. This location may provide an RNG schema
override, otherwise the RNG schema from the system location
will be used.

=item B<user>

This is the user private database location, intended for use
by unprivileged local users wishing to provide applications
they use with an updated database. This location may provide
an RNG schema override, otherwise the RNG schema from the
local location will be used, or failing that the system
location.

=back

If run by a privileged account (ie root), the B<local> database
location will be validate by default, otherwise the B<user> location
will be validated.

Alternatively it is possible to directly provide a list of files
to be validated using the (C<LOCAL-PATH1> or C<URI1>) arguments.

Any validation errors will be displayed on the console when
detected.

=head1 OPTIONS

=over 8

=item B<--user>

Override the default behaviour to force validating files from the
B<user> database location.

=item B<--local>

Override the default behaviour to force validating files from the
B<local> database location.

=item B<--system>

Override the default behaviour to force validating files from the
B<system> database location.

=item B<--dir=PATH>

Override the default behaviour to force validating files from the
custom directory B<PATH>.

=item B<--root=PATH>

Prefix the database location with the root directory given by
C<PATH>. This is useful when wishing to validate files that are
in a chroot environment or equivalent.

=item B<-v>, B<--verbose>

Display verbose progress information when validating files

=back

=head1 EXIT STATUS

The exit status will be 0 if all files passed validation,
or 1 if a validation error was hit.

=head1 SEE ALSO

C<osinfo-db-path(1)>

=head1 AUTHORS

Daniel P. Berrange <berrange@redhat.com>

=head1 COPYRIGHT

Copyright (C) 2012-2016 Red Hat, Inc.

=head1 LICENSE

C<osinfo-db-validate> is distributed under the terms of the GNU LGPL v2+
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
