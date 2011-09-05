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
 * \addtogroup inputbox
 * @{
 */

static void inputbox_activate_cb (Inputbox *obj, gpointer unused);
static gboolean inputbox_keypress_cb (Inputbox *obj, GdkEvent *event, gpointer unused);
static void inputbox_focus_in_cb (Inputbox *obj, GdkEvent *event, gpointer unused);
static void inputbox_focus_out_cb (Inputbox *obj, GdkEvent *event, gpointer unused);

static void inputbox_check_mode (Inputbox *obj);

G_DEFINE_TYPE (Inputbox, inputbox, GTK_TYPE_ENTRY)

/* Constructors */

/*!
 * \public \memberof Inputbox
 * \fn GtkWidget *inputbox_new (void)
 * @return A new #Inputbox object.
 *
 * Create a new #Inputbox object.
 */
GtkWidget *inputbox_new (void)
{
     Inputbox *obj = g_object_new (CREAM_TYPE_INPUTBOX, NULL);
     g_signal_connect (G_OBJECT (obj), "activate", G_CALLBACK (inputbox_activate_cb), NULL);
     g_signal_connect (G_OBJECT (obj), "key-press-event", G_CALLBACK (inputbox_keypress_cb), NULL);
     g_signal_connect (G_OBJECT (obj), "focus-in-event", G_CALLBACK (inputbox_focus_in_cb), NULL);
     g_signal_connect (G_OBJECT (obj), "focus-out-event", G_CALLBACK (inputbox_focus_out_cb), NULL);
     return GTK_WIDGET (obj);
}

static void inputbox_class_init (InputboxClass *klass)
{
     return;
}

static void inputbox_init (Inputbox *self)
{
     self->history = NULL;
     self->current = NULL;
}

/* callbacks */

static void inputbox_activate_cb (Inputbox *obj, gpointer unused)
{
     GError *error = NULL;
     WebView *fwebview;
     gchar *txt;

     g_return_if_fail (gtk_entry_get_text_length (GTK_ENTRY (obj)) > 0);
     txt = g_strdup (gtk_entry_get_text (GTK_ENTRY (obj)));
     gtk_entry_set_text (GTK_ENTRY (obj), "");

     fwebview = CREAM_WEBVIEW (notebook_get_focus (CREAM_NOTEBOOK (gtk_vim_split_get_focus (GTK_VIM_SPLIT (global.gui.vimsplit)))));

     switch (txt[0])
     {
          case ':':
               if (!run_command (txt + 1, &error))
               {
                    gtk_entry_set_text (GTK_ENTRY (obj), error->message);
                    g_error_free (error);
               }

               break;

          case '/':
               /* search forward */
               if (!cream_module_search (CREAM_MODULE (webview_get_module (fwebview)), fwebview->child, txt + 1, TRUE))
               {
                    gchar *err = g_strdup_printf (_("No matches found for: %s"), txt + 1);
                    gtk_entry_set_text (GTK_ENTRY (obj), err);
                    g_free (err);
               }
               break;

          case '?':
               /* search backward */
               if (!cream_module_search (CREAM_MODULE (webview_get_module (fwebview)), fwebview->child, txt + 1, FALSE))
               {
                    gchar *err = g_strdup_printf (_("No matches found for: %s"), txt + 1);
                    gtk_entry_set_text (GTK_ENTRY (obj), err);
                    g_free (err);
               }
               break;

          default: break;
     }

     gtk_editable_set_position (GTK_EDITABLE (obj), -1);

     obj->history = g_list_prepend (obj->history, txt);
}

static gboolean inputbox_keypress_cb (Inputbox *obj, GdkEvent *event, gpointer unused)
{
     GdkEventKey ekey = event->key;
     gchar *key = gdk_keyval_name (ekey.keyval);
     gboolean ret = TRUE;

     if (g_str_equal (key, "Escape"))
     {
          gtk_entry_set_text (GTK_ENTRY (obj), "");
          inputbox_check_mode (obj);
     }
     else if (g_str_equal (key, "Tab"))
     {
          /* completion */;
     }
     else if (g_str_equal (key, "Up") && obj->history != NULL)
     {
          obj->current = (obj->current == NULL ? obj->history : obj->current->next);

          if (obj->current != NULL)
          {
               gtk_entry_set_text (GTK_ENTRY (obj), (gchar *) obj->current->data);
               inputbox_check_mode (obj);
          }
     }
     else if (g_str_equal (key, "Down") && obj->history != NULL)
     {
          obj->current = (obj->current == NULL ? obj->history : obj->current->prev);

          if (obj->current != NULL)
          {
               gtk_entry_set_text (GTK_ENTRY (obj), (gchar *) obj->current->data);
               inputbox_check_mode (obj);
          }
     }
     else if (g_str_equal (key, "BackSpace"))
     {
          const gchar *txt = gtk_entry_get_text (GTK_ENTRY (obj));

          if (strlen (txt) == 1)
          {
               statusbar_set_state (CREAM_STATUSBAR (global.gui.statusbar), CREAM_MODE_NORMAL);
               gtk_grab_remove (GTK_WIDGET (obj));
          }

          ret = FALSE;
     }
     else
     {
          inputbox_check_mode (obj);
          ret = FALSE;
     }

     if (ret) gtk_editable_set_position (GTK_EDITABLE (obj), -1);

     return ret;
}

static void inputbox_focus_in_cb (Inputbox *obj, GdkEvent *event, gpointer unused)
{
     statusbar_set_state (CREAM_STATUSBAR (global.gui.statusbar), CREAM_MODE_COMMAND);
}

static void inputbox_focus_out_cb (Inputbox *obj, GdkEvent *event, gpointer unused)
{
     statusbar_set_state (CREAM_STATUSBAR (global.gui.statusbar), CREAM_MODE_NORMAL);
}

/* methods */

static void inputbox_check_mode (Inputbox *obj)
{
     const gchar *txt = gtk_entry_get_text (GTK_ENTRY (obj));

     if (strlen (txt) > 0)
     {
          if (txt[0] == '?' || txt[0] == '/')
          {
               statusbar_set_state (CREAM_STATUSBAR (global.gui.statusbar), CREAM_MODE_SEARCH);
               gtk_grab_add (GTK_WIDGET (obj));
          }
          else
               gtk_grab_remove (GTK_WIDGET (obj));
     }
     else
     {
          statusbar_set_state (CREAM_STATUSBAR (global.gui.statusbar), CREAM_MODE_COMMAND);
          gtk_grab_remove (GTK_WIDGET (obj));
     }
}

/*! @} */
