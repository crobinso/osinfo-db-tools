/*
 * Copyright (C) 2016 Red Hat, Inc.
 *
 * osinfo-db-util: misc helper APIs
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

#include <glib/gi18n.h>

#include "osinfo-db-util.h"

GQuark osinfo_db_error_quark(void)
{
    return g_quark_from_static_string("osinfo-db-error");
}

GFile *osinfo_db_get_system_path(const gchar *root)
{
    GFile *file;
    g_autofree gchar *dbdir = NULL;
    const gchar *path = g_getenv("OSINFO_SYSTEM_DIR");
    if (!path)
        path = DATA_DIR "/osinfo";

    dbdir = g_build_filename(root, path, NULL);
    file = g_file_new_for_path(dbdir);
    return file;
}


GFile *osinfo_db_get_local_path(const gchar *root)
{
    GFile *file;
    g_autofree gchar *dbdir = NULL;
    const gchar *path = g_getenv("OSINFO_LOCAL_DIR");
    if (!path)
        path = SYSCONFDIR "/osinfo";

    dbdir = g_build_filename(root, path, NULL);
    file = g_file_new_for_path(dbdir);
    return file;
}


GFile *osinfo_db_get_user_path(const gchar *root)
{
    GFile *file;
    g_autofree gchar *dbdir = NULL;
    const gchar *path = g_getenv("OSINFO_USER_DIR");
    const gchar *configdir = g_get_user_config_dir();

    if (path) {
        dbdir = g_build_filename(root, path, NULL);
    } else {
        dbdir = g_build_filename(root, configdir, "osinfo", NULL);
    }
    file = g_file_new_for_path(dbdir);
    return file;
}


GFile *osinfo_db_get_custom_path(const gchar *dir,
                                 const gchar *root)
{
    GFile *file;
    g_autofree gchar *dbdir = NULL;

    dbdir = g_build_filename(root, dir, NULL);
    file = g_file_new_for_path(dbdir);
    return file;
}


GFile *osinfo_db_get_path(const char *root,
                          gboolean user,
                          gboolean local,
                          gboolean system,
                          const char *custom)
{
    if (custom) {
        return osinfo_db_get_custom_path(custom, root);
    } else if (user) {
        return osinfo_db_get_user_path(root);
    } else if (local) {
        return osinfo_db_get_local_path(root);
    } else if (system) {
        return osinfo_db_get_system_path(root);
#ifndef WIN32
    } else if (geteuid() == 0) {
        return osinfo_db_get_local_path(root);
#endif
    } else {
        return osinfo_db_get_user_path(root);
    }
}

GFile *osinfo_db_get_file(const char *root,
                          gboolean user,
                          gboolean local,
                          gboolean system,
                          const char *custom,
                          const gchar *file,
                          GError **err)
{
    GFile *ret = NULL;
    gboolean tryAll = TRUE;
    GFile *paths[4];
    gsize npaths = 0;
    gsize i;

    if (user || local || system || custom)
        tryAll = FALSE;

    if (custom)
        paths[npaths++] = osinfo_db_get_custom_path(custom, root);

    if (tryAll || user)
        paths[npaths++] = osinfo_db_get_user_path(root);

    if (tryAll || local)
        paths[npaths++] = osinfo_db_get_local_path(root);

    if (tryAll || system)
        paths[npaths++] = osinfo_db_get_system_path(root);

    for (i = 0; i < npaths; i++) {
        ret = g_file_resolve_relative_path(paths[i], file);
        if (g_file_query_exists(ret, NULL))
            break;
        g_clear_object(&ret);
    }

    if (!ret) {
        g_set_error(err, OSINFO_DB_ERROR, 0,
                    _("Unable to locate '%s' in any database location"),
                    file);
    }

    return ret;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
