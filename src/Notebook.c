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
 * \addtogroup notebook
 * @{
 */

static void notebook_switch_page_cb (Notebook *self, GtkWidget *webview, guint page_num, gpointer unused);

static void notebook_signal_title_changed_cb (WebView *webview, const gchar *title, Notebook *obj);
static void notebook_signal_grab_focus_cb (WebView *webview, Notebook *obj);

G_DEFINE_TYPE (Notebook, notebook, GTK_TYPE_NOTEBOOK);

/*!
 * \public \memberof Notebook
 * \fn GtkWidget *notebook_new (void)
 * @return A new #Notebook object.
 *
 * Create a new #Notebook object.
 */
GtkWidget *notebook_new (void)
{
     Notebook *obj = g_object_new (notebook_get_type (), NULL);

     g_signal_connect (G_OBJECT (obj), "switch-page", G_CALLBACK (notebook_switch_page_cb), NULL);

     return GTK_WIDGET (obj);
}

static void notebook_class_init (NotebookClass *klass)
{
     return;
}

static void notebook_init (Notebook *obj)
{
     obj->focus = NULL;
     obj->webviews = NULL;

     gtk_notebook_set_scrollable (GTK_NOTEBOOK (obj), TRUE);
     gtk_notebook_popup_enable (GTK_NOTEBOOK (obj));
}

/* methods */

/*!
 * \public \memberof Notebook
 * \fn GtkWidget *notebook_get_focus (Notebook *obj)
 * @param obj A #Notebook object.
 * @return A #WebView object.
 *
 * Return the current focused webview.
 */
GtkWidget *notebook_get_focus (Notebook *obj)
{
     g_return_val_if_fail (CREAM_IS_NOTEBOOK (obj), NULL);
     return obj->focus;
}

/*!
 * \public \memberof Notebook
 * \fn void notebook_open (Notebook *obj, const gchar *url)
 * @param obj A #Notebook object.
 * @param url URL to load.
 *
 * Open URL in the current webview.
 */
void notebook_open (Notebook *obj, const gchar *url)
{
     GtkWidget *webview;

     g_return_if_fail (CREAM_IS_NOTEBOOK (obj));
     g_return_if_fail (url != NULL);

     if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (obj)) == 0)
          notebook_tabopen (obj, url);
     else
     {
          webview = gtk_notebook_get_nth_page (GTK_NOTEBOOK (obj),
               gtk_notebook_get_current_page (GTK_NOTEBOOK (obj))
          );

          webview_load_uri (CREAM_WEBVIEW (webview), url);
     }
}

/*!
 * \public \memberof Notebook
 * \fn void notebook_tabopen (Notebook *obj, const gchar *url)
 * @param obj A #Notebook object.
 * @param url URL to load.
 *
 * Open URL in a new webview.
 */
void notebook_tabopen (Notebook *obj, const gchar *url)
{
     GObject *module;
     GtkWidget *webview;
     UriScheme u;

     g_return_if_fail (CREAM_IS_NOTEBOOK (obj));
     g_return_if_fail (uri_scheme_parse (&u, url));

     module = get_protocol (u.scheme);
     webview = webview_new (module);
     CREAM_WEBVIEW (webview)->notebook = GTK_WIDGET (obj);
     webview_load_uri (CREAM_WEBVIEW (webview), url);

     gtk_notebook_append_page_menu (GTK_NOTEBOOK (obj), webview,
               NULL,
               NULL
     );

     g_signal_connect (G_OBJECT (webview), "module-changed", G_CALLBACK (ui_show), NULL);
     g_signal_connect (G_OBJECT (webview), "grab-focus",     G_CALLBACK (notebook_signal_grab_focus_cb), obj);
     g_signal_connect (G_OBJECT (webview), "title-changed",  G_CALLBACK (notebook_signal_title_changed_cb), obj);

     gtk_widget_grab_focus (webview);
}

/*!
 * \public \memberof Notebook
 * \fn void notebook_close (Notebook *obj, gint page)
 * @param obj A #Notebook object.
 * @param page The page to close.
 *
 * Close a tab in the notebook.
 */
void notebook_close (Notebook *obj, gint page)
{
     GtkWidget *webview = g_object_ref (gtk_notebook_get_nth_page (GTK_NOTEBOOK (obj), page));
     GList *node = g_list_find (obj->webviews, webview);
     obj->webviews = g_list_remove_link (obj->webviews, node);
     gtk_notebook_remove_page (GTK_NOTEBOOK (obj), page);
}

/* signals */

static void notebook_switch_page_cb (Notebook *self, GtkWidget *webview, guint page_num, gpointer unused)
{
     self->focus = webview;
     g_signal_emit_by_name (G_OBJECT (self), "grab-focus");
}

static void notebook_signal_title_changed_cb (WebView *webview, const gchar *title, Notebook *obj)
{
     gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (obj), GTK_WIDGET (webview), title);
}

static void notebook_signal_grab_focus_cb (WebView *webview, Notebook *obj)
{
     obj->focus = GTK_WIDGET (webview);
     gtk_notebook_set_current_page (GTK_NOTEBOOK (obj), gtk_notebook_page_num (GTK_NOTEBOOK (obj), obj->focus));
     g_signal_emit_by_name (G_OBJECT (obj), "grab-focus");
}

/*! @} */
