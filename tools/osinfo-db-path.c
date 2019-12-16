/*
 * Copyright (C) 2016 Red Hat, Inc
 *
 * osinfo-db-path: report the path to the osinfo database
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
#include <stdlib.h>

#include "osinfo-db-util.h"

const char *argv0;

gint main(gint argc, gchar **argv)
{
    g_autoptr(GOptionContext) context = NULL;
    g_autoptr(GError) error = NULL;
    g_autoptr(GFile) dir = NULL;
    g_autofree gchar *path = NULL;
    gboolean user = FALSE;
    gboolean local = FALSE;
    gboolean system = FALSE;
    const gchar *root = "";
    const gchar *custom = NULL;
    int locs = 0;
    const GOptionEntry entries[] = {
      { "user", 0, 0, G_OPTION_ARG_NONE, (void *)&user,
        N_("Report the user directory"), NULL, },
      { "local", 0, 0, G_OPTION_ARG_NONE, (void *)&local,
        N_("Report the local directory"), NULL, },
      { "system", 0, 0, G_OPTION_ARG_NONE, (void *)&system,
        N_("Report the system directory"), NULL, },
      { "dir", 0, 0, G_OPTION_ARG_STRING, (void *)&custom,
        N_("Report the custom directory"), NULL, },
      { "root", 0, 0, G_OPTION_ARG_STRING, &root,
        N_("Report against root directory"), NULL, },
      { NULL, 0, 0, 0, NULL, NULL, NULL },
    };
    argv0 = argv[0];

    setlocale(LC_ALL, "");
    textdomain(GETTEXT_PACKAGE);
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    context = g_option_context_new(_("- Report database locations "));

    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("%s: error while parsing commandline options: %s\n\n"),
                   argv0, error->message);
        g_printerr("%s\n", g_option_context_get_help(context, FALSE, NULL));
        return EXIT_FAILURE;
    }

    if (argc > 1) {
        g_printerr(_("%s: unexpected extra arguments\n"),
                   argv0);
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

    dir = osinfo_db_get_path(root, user, local, system, custom);

    path = g_file_get_path(dir);
    g_print("%s\n", path);

    return EXIT_SUCCESS;
}


/*
=pod

=head1 NAME

osinfo-db-path - Report database locations

=head1 SYNOPSIS

osinfo-db-path [OPTIONS...]

=head1 DESCRIPTION

The B<osinfo-db-path> tool will report the paths associated
with the standard osinfo database locations:

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
location will be reported by default, otherwise the B<user> location
will be reported.

=head1 OPTIONS

=over 8

=item B<--user>

Override the default behaviour to force reporting of the
B<user> database location.

=item B<--local>

Override the default behaviour to force reporting of the
B<local> database location.

=item B<--system>

Override the default behaviour to force reporting of the
B<system> database location.

=item B<--dir=PATH>

Override the default behaviour to force reporting of the
custom directory B<PATH>.

=item B<--root=PATH>

Prefix the database location with the root directory given
by C<PATH>. This is useful when wishing to report paths
relative to a chroot environment or equivalent.

=back

=head1 EXIT STATUS

The exit status will be 0 if the requested path was reported,
or 1 if the arguments were invalid.

=head1 SEE ALSO

C<osinfo-db-export(1)>, C<osinfo-db-import(1)>

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
