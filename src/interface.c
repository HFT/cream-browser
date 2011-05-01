/*
 * Copyright © 2011, David Delassus <david.jose.delassus@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "local.h"

/*!
 * \addtogroup interface
 * @{
 */

static void window_destroy (GtkWidget *window, gpointer data)
{
     exit (EXIT_SUCCESS);
}

/*!
 * \fn void ui_init (void)
 * Create the main window.
 */
void ui_init (void)
{
     global.gui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title (GTK_WINDOW (global.gui.window), PACKAGE);
     g_signal_connect (G_OBJECT (global.gui.window), "destroy", G_CALLBACK (window_destroy), NULL);
}

/*!
 * \fn void ui_show (void)
 * Show/Redraw the main window.
 */
void ui_show (void)
{
     gtk_widget_show_all (global.gui.window);
}

/*! @} */

/*!
 * \defgroup lua-interface Graphical User Interface
 * \ingroup lua
 * LUA bindings for the GUI.
 *
 * @{
 */

static int luaL_signal_connect (lua_State *L)
{
     const gchar *signal;
     gpointer obj;
     int ref;

     luaL_checktype (L, 1, LUA_TUSERDATA);
     signal = luaL_checkstring (L, 2);
     ref    = luaL_checkfunction (L, 3);

     return 0;
}

static int luaL_gui_box_new (lua_State *L)
{
     g_return_val_if_fail (global.theme, 0);

     if (global.theme->global.box.horizontal)
          global.gui.box = gtk_hbox_new (global.theme->global.box.homogeneous, global.theme->global.box.padding);
     else
          global.gui.box = gtk_vbox_new (global.theme->global.box.homogeneous, global.theme->global.box.padding);

     gtk_container_add (GTK_CONTAINER (global.gui.window), global.gui.box);
     return 0;
}

static int luaL_gui_box_pack (lua_State *L)
{
     gpointer obj;

     g_return_val_if_fail (global.theme, 0);
     luaL_checktype (L, 1, LUA_TUSERDATA);
     if ((obj = luaL_checkudata (L, 1, LUA_TNOTEBOOK)))
     {
          GtkWidget *w = *((GtkWidget **) obj);

          if (global.theme->tab.box.pack_start)
          {
               gtk_box_pack_start (GTK_BOX (global.gui.box), w, global.theme->tab.box.expand,
                                                                global.theme->tab.box.fill,
                                                                global.theme->tab.box.padding);
          }
          else
          {
               gtk_box_pack_end (GTK_BOX (global.gui.box), w, global.theme->tab.box.expand,
                                                              global.theme->tab.box.fill,
                                                              global.theme->tab.box.padding);
          }
     }
     else if ((obj = luaL_checkudata (L, 1, LUA_TVIEWAREA)))
     {
          GtkWidget *w = *((GtkWidget **) obj);

          if (global.theme->webview.box.pack_start)
          {
               gtk_box_pack_start (GTK_BOX (global.gui.box), w, global.theme->webview.box.expand,
                                                                global.theme->webview.box.fill,
                                                                global.theme->webview.box.padding);
          }
          else
          {
               gtk_box_pack_end (GTK_BOX (global.gui.box), w, global.theme->webview.box.expand,
                                                              global.theme->webview.box.fill,
                                                              global.theme->webview.box.padding);
          }
     }
     else if ((obj = luaL_checkudata (L, 1, LUA_TSTATUSBAR)))
     {
          GtkWidget *w = *((GtkWidget **) obj);

          if (global.theme->statusbar.box.pack_start)
          {
               gtk_box_pack_start (GTK_BOX (global.gui.box), w, global.theme->statusbar.box.expand,
                                                                global.theme->statusbar.box.fill,
                                                                global.theme->statusbar.box.padding);
          }
          else
          {
               gtk_box_pack_end (GTK_BOX (global.gui.box), w, global.theme->statusbar.box.expand,
                                                              global.theme->statusbar.box.fill,
                                                              global.theme->statusbar.box.padding);
          }

     }
     else if ((obj = luaL_checkudata (L, 1, LUA_TPROMPTBOX)))
     {
          GtkWidget *w = *((GtkWidget **) obj);

          if (global.theme->promptbox.box.pack_start)
          {
               gtk_box_pack_start (GTK_BOX (global.gui.box), w, global.theme->promptbox.box.expand,
                                                                global.theme->promptbox.box.fill,
                                                                global.theme->promptbox.box.padding);
          }
          else
          {
               gtk_box_pack_end (GTK_BOX (global.gui.box), w, global.theme->promptbox.box.expand,
                                                              global.theme->promptbox.box.fill,
                                                              global.theme->promptbox.box.padding);
          }
     }
     else
          luaL_typerror (L, 1, "userdata");

     return 0;
}



/*! @} */
