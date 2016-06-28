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
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Daniel P. Berrange <berrange@redhat.com>
 */

#ifndef OSINFO_DB_UTIL_H__
#define OSINFO_DB_UTIL_H__

#include <gio/gio.h>

GFile *osinfo_db_get_system_path(const gchar *root);
GFile *osinfo_db_get_local_path(const gchar *root);
GFile *osinfo_db_get_user_path(const gchar *root);
GFile *osinfo_db_get_custom_path(const gchar *dir,
                                 const gchar *root);
GFile *osinfo_db_get_path(const char *root,
                          gboolean user,
                          gboolean local,
                          gboolean system,
                          const char *custom);

#endif /* OSINFO_DB_UTIL_H__ */

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */