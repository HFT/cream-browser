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


#include "local.h"

gboolean control_socket (GIOChannel *channel)
{
     struct sockaddr_un remote;
     unsigned int t = sizeof (remote);
     int csock;
     GIOChannel *cchannel;

     csock = accept (g_io_channel_unix_get_fd (channel),
                     (struct sockaddr *) &remote, &t);

     if ((cchannel = g_io_channel_unix_new (csock)))
     {
          g_io_add_watch (cchannel, G_IO_IN | G_IO_HUP, (GIOFunc) control_client_socket, cchannel);
     }

     return TRUE;
}

gboolean control_client_socket (GIOChannel *channel)
{
     GString *result = g_string_new ("\n");
     GError *error = NULL;
     GIOStatus ret;
     char *line;
     gsize len;

     ret = g_io_channel_read_line (channel, &line, &len, NULL, &error);
     if (ret == G_IO_STATUS_ERROR)
     {
          g_warning ("Error reading UNIX socket '%s' : %s", global.unix_sock.path, error->message);
          g_io_channel_shutdown (channel, TRUE, NULL);
          g_error_free (error);
          return FALSE;
     }
     else if (ret == G_IO_STATUS_EOF)
     {
          /* shutdown and remove channel watch from main loop */
          g_io_channel_shutdown (channel, TRUE, NULL);
          return FALSE;
     }

     if (line)
     {
          run_command (line, &result, get_current_creamtabbed ());

          ret = g_io_channel_write_chars (channel, result->str, result->len, &len, &error);
          if (ret == G_IO_STATUS_ERROR)
          {
               g_warning ("Error writing UNIX socket '%s' : %s\n", global.unix_sock.path, error->message);
               g_error_free (error);
          }
          g_io_channel_flush (channel, NULL);
     }

     g_string_free (result, TRUE);
     g_free (line);
     return TRUE;
}

void cb_cream_destroy (GtkWidget *emit, gpointer data)
{
     cream_release (EXIT_SUCCESS);
}

void cb_cream_update_notebook_title (GtkWidget *child, gpointer data)
{
     gtk_notebook_set_tab_label (global.notebook, child, cream_create_tab_label (CREAM_TABBED (child)));
}

void cb_cream_notebook_close_page (GtkButton *button, GtkWidget *child)
{
     gtk_notebook_remove_page (global.notebook,
          gtk_notebook_page_num (global.notebook, child)
     );

     if (gtk_notebook_get_n_pages (global.notebook) == 0)
          cream_release (EXIT_SUCCESS);
}

gboolean cb_inputbox_keys (GtkEntry *inputbox, GdkEventKey *event, CreamTabbed *obj)
{
     gboolean ret = TRUE;

     if (global.browser.mode == CmdMode && event->keyval == GDK_Escape)
     {
          echo (obj, "");
          global.browser.mode = BindMode;
          return ret;
     }
     else
          ret = bind_getkey (CREAM_VIEW (obj->creamview), event, obj);

     if (!ret)
     {
          switch (event->keyval)
          {
               case GDK_Up:
                    /*! \todo Go up in the commands' history */
                    break;

               case GDK_Down:
                    /*! \todo Go down in the commands' history */
                    break;

               case GDK_Tab:
                    /*! \todo Completion (next) */
                    break;

               case GDK_ISO_Left_Tab:
                    /*! \todo Completion (previous) */
                    break;

               default: ret = FALSE; break;
          }
     }

     return ret;
}
