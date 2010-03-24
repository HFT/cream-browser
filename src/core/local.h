/*
 *        Copyright (c) 2010, David Delassus <linkdd@ydb.me>
 *
 *        This file is part of Cream-Browser
 *
 *        Cream-Browser is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation, either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        Cream-Browser is distributed in the hope that it will be useful
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with Cream-Browser. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LOCAL_H
#define __LOCAL_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>
#include <curl/curl.h>
#include <gnet.h>

#include "CreamTabbed.h"

#include "structs.h"
#include "interface.h"
#include "callbacks.h"
#include "command.h"

#define CREAM_FILE(path)      g_build_filename (g_get_home_dir (), ".config", "cream-browser", path, NULL)

extern struct global_t global;

gboolean cream_init (int *argc, char ***argv, GError **error);
void cream_release (int exit_code);

#endif /* __LOCAL_H */
