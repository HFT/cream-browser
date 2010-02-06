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

#include "WebViewModule.h"
#include <marshal.h>

enum
{
     URI_CHANGED_SIGNAL,
     NEW_TITLE_SIGNAL,
     STATUS_CHANGED_SIGNAL,
     NEW_DOWNLOAD_SIGNAL,
     SWITCH_MODULE_SIGNAL,
     NB_SIGNALS
};

static guint module_web_view_signals[NB_SIGNALS] = { 0 };

static void module_web_view_class_init (ModuleWebViewClass *class);
static void module_web_view_init (ModuleWebView *obj);

static WebKitWebView *module_web_view_cb_create_inspector_win (WebKitWebInspector *inspector, WebKitWebView *view, gpointer data);
static void module_web_view_cb_uri_changed (ModuleWebView *webview, GParamSpec *arg1, gpointer data);
static void module_web_view_cb_title_changed (ModuleWebView *webview, WebKitWebFrame *frame, gchar *title, gpointer data);
static void module_web_view_cb_load_progress_changed (ModuleWebView *webview, gint progress, gpointer data);
static void module_web_view_cb_load_committed (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data);
static void module_web_view_cb_load_finished (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data);
static WebKitNavigationResponse module_web_view_cb_navigation (ModuleWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, gpointer data);
static WebKitWebView *module_web_view_cb_create_web_view (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data);
static gboolean module_web_view_cb_mimetype (ModuleWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, gchar *mimetype, WebKitWebPolicyDecision *decision, gpointer data);
static gboolean module_web_view_cb_download (ModuleWebView *webview, WebKitDownload *download, gpointer data);
static void module_web_view_cb_hoverlink (ModuleWebView *webview, gchar *title, gchar *link, gpointer data);

GtkType module_web_view_get_type (void)
{
     static GtkType module_web_view_type = 0;

     if (!module_web_view_type)
     {
          static const GtkTypeInfo module_web_view_info =
          {
               "ModuleWebView",
               sizeof (ModuleWebView),
               sizeof (ModuleWebViewClass),
               (GtkClassInitFunc) module_web_view_class_init,
               (GtkObjectInitFunc) module_web_view_init,
               NULL, NULL,
               (GtkClassInitFunc) NULL
          };

          module_web_view_type = gtk_type_unique (WEBKIT_TYPE_WEB_VIEW, &module_web_view_info);
     }

     return module_web_view_type;
}

static void module_web_view_class_init (ModuleWebViewClass *class)
{
     module_web_view_signals[URI_CHANGED_SIGNAL] = g_signal_new (
          "uri-changed",
          G_TYPE_FROM_CLASS (class),
          G_SIGNAL_RUN_LAST,
          G_STRUCT_OFFSET (ModuleWebViewClass, uri_changed),
          NULL, NULL,
          g_cclosure_marshal_VOID__STRING,
          G_TYPE_NONE,
          1, G_TYPE_STRING);

     module_web_view_signals[NEW_TITLE_SIGNAL] = g_signal_new (
          "new-title",
          G_TYPE_FROM_CLASS (class),
          G_SIGNAL_RUN_LAST,
          G_STRUCT_OFFSET (ModuleWebViewClass, new_title),
          NULL, NULL,
          g_cclosure_marshal_VOID__STRING,
          G_TYPE_NONE,
          1, G_TYPE_STRING);

     module_web_view_signals[STATUS_CHANGED_SIGNAL] = g_signal_new (
          "status-changed",
          G_TYPE_FROM_CLASS (class),
          G_SIGNAL_RUN_LAST,
          G_STRUCT_OFFSET (ModuleWebViewClass, status_changed),
          NULL, NULL,
          g_cclosure_marshal_VOID__STRING,
          G_TYPE_NONE,
          1, G_TYPE_STRING);

     module_web_view_signals[NEW_DOWNLOAD_SIGNAL] = g_signal_new (
          "new-download",
          G_TYPE_FROM_CLASS (class),
          G_SIGNAL_RUN_LAST,
          G_STRUCT_OFFSET (ModuleWebViewClass, new_download),
          NULL, NULL,
          g_cclosure_user_marshal_BOOLEAN__OBJECT,
          G_TYPE_BOOLEAN,
          1, G_TYPE_OBJECT);

     module_web_view_signals[SWITCH_MODULE_SIGNAL] = g_signal_new (
          "switch-module",
          G_TYPE_FROM_CLASS (class),
          G_SIGNAL_RUN_LAST,
          G_STRUCT_OFFSET (ModuleWebViewClass, switch_module),
          NULL, NULL,
          g_cclosure_user_marshal_BOOLEAN__STRING,
          G_TYPE_BOOLEAN,
          1, G_TYPE_STRING);
}

static void module_web_view_init (ModuleWebView *obj)
{
     obj->uri = NULL;
     obj->title = NULL;
     obj->status = NULL;
     obj->jsmsg = NULL;
     obj->inspector = NULL;
     obj->settings = NULL;
     obj->ico = NULL;
}

GtkWidget *module_web_view_new (void)
{
     ModuleWebView *obj = gtk_type_new (module_web_view_get_type ());

     obj->settings = webkit_web_settings_new ();
     g_object_set (G_OBJECT (obj->settings), "enable-developer-extras", TRUE, NULL);
     webkit_web_view_set_settings (WEBKIT_WEB_VIEW (obj), obj->settings);

     obj->inspector = webkit_web_view_get_inspector (WEBKIT_WEB_VIEW (obj));
     g_object_set (G_OBJECT (obj->inspector), "javascript-profiling-enabled", TRUE, NULL);
     g_signal_connect (G_OBJECT (obj->inspector), "inspect-web-view", G_CALLBACK (module_web_view_cb_create_inspector_win), obj);

     g_object_connect (G_OBJECT (obj),
          "signal::notify::uri",                            G_CALLBACK (module_web_view_cb_uri_changed),           NULL,
          "signal::title-changed",                          G_CALLBACK (module_web_view_cb_title_changed),         NULL,
          "signal::load-progress-changed",                  G_CALLBACK (module_web_view_cb_load_progress_changed), NULL,
          "signal::load-committed",                         G_CALLBACK (module_web_view_cb_load_committed),        NULL,
          "signal::load-finished",                          G_CALLBACK (module_web_view_cb_load_finished),         NULL,
          "signal::navigation-requested",                   G_CALLBACK (module_web_view_cb_navigation),            NULL,
          "signal::create-web-view",                        G_CALLBACK (module_web_view_cb_create_web_view),       NULL,
          "signal::mime-type-policy-decision-requested",    G_CALLBACK (module_web_view_cb_mimetype),              NULL,
          "signal::download-requested",                     G_CALLBACK (module_web_view_cb_download),              NULL,
          "signal::hovering-over-link",                     G_CALLBACK (module_web_view_cb_hoverlink),             NULL,
     NULL);

     return GTK_WIDGET (obj);
}

/* WebKit signals */
static WebKitWebView *module_web_view_cb_create_inspector_win (WebKitWebInspector *inspector, WebKitWebView *view, gpointer data)
{
     GtkWidget *win;
     GtkWidget *scroll;
     GtkWidget *page;

     win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title (GTK_WINDOW (win), g_strconcat (
          "WebInspector - ", webkit_web_view_get_uri (view), NULL)
     );

     scroll = gtk_scrolled_window_new (NULL, NULL);
     gtk_container_add (GTK_CONTAINER (win), scroll);

     page = webkit_web_view_new ();
     gtk_container_add (GTK_CONTAINER (scroll), page);

     gtk_widget_show_all (win);

     return WEBKIT_WEB_VIEW (page);
}

static void module_web_view_cb_uri_changed (ModuleWebView *webview, GParamSpec *arg1, gpointer data)
{
     if (webview->uri != NULL)
          g_free (webview->uri);
     webview->uri = g_strdup (webkit_web_view_get_uri (WEBKIT_WEB_VIEW (webview)));
     webview->ico = favicon_new (webkit_web_view_get_icon_uri (WEBKIT_WEB_VIEW (webview)));

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[URI_CHANGED_SIGNAL],
          0,
          webview->uri
     );
}

static void module_web_view_cb_title_changed (ModuleWebView *webview, WebKitWebFrame *frame, gchar *title, gpointer data)
{
     if (webview->title != NULL)
          g_free (webview->title);
     webview->title = g_strdup (title);

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[NEW_TITLE_SIGNAL],
          0,
          webview->title
     );
}

static void module_web_view_cb_load_progress_changed (ModuleWebView *webview, gint progress, gpointer data)
{
     if (webview->status != NULL)
          g_free (webview->status);
     webview->status = g_strdup_printf ("Transfering data from %s (%d%%)", webview->uri, progress);

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[STATUS_CHANGED_SIGNAL],
          0,
          webview->status
     );
}

static void module_web_view_cb_load_committed (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data)
{
     if (webview->status != NULL)
          g_free (webview->status);
     webview->status = g_strdup_printf ("Waiting reply from %s", webview->uri);

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[STATUS_CHANGED_SIGNAL],
          0,
          webview->status
     );
}

static void module_web_view_cb_load_finished (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data)
{
     if (webview->status != NULL)
          g_free (webview->status);
     webview->status = g_strdup (webview->uri);

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[STATUS_CHANGED_SIGNAL],
          0,
          webview->status
     );
}

static WebKitNavigationResponse module_web_view_cb_navigation (ModuleWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, gpointer data)
{
     const gchar *uri = webkit_network_request_get_uri (request);

     if (!g_str_has_prefix (uri, "http://") && !g_str_has_prefix (uri, "https://"))
     {
          gboolean ret = FALSE;

          g_signal_emit (
               G_OBJECT (webview),
               module_web_view_signals[SWITCH_MODULE_SIGNAL],
               0,
               uri,
               &ret
          );

          return (ret ? WEBKIT_NAVIGATION_RESPONSE_IGNORE : WEBKIT_NAVIGATION_RESPONSE_ACCEPT);
     }
     else if (g_str_has_prefix (uri, "javascript:"))
     {
          module_web_view_js_script_execute (webview, &uri[11]);
          return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
     }
     else
     {
          return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;
     }
}

static WebKitWebView *module_web_view_cb_create_web_view (ModuleWebView *webview, WebKitWebFrame *frame, gpointer data)
{
     return WEBKIT_WEB_VIEW (module_web_view_new ());
}

static gboolean module_web_view_cb_mimetype (ModuleWebView *webview, WebKitWebFrame *frame, WebKitNetworkRequest *request, gchar *mimetype, WebKitWebPolicyDecision *decision, gpointer data)
{
     if (webkit_web_view_can_show_mime_type (WEBKIT_WEB_VIEW (webview), mimetype))
          webkit_web_policy_decision_use (decision);
     else
          webkit_web_policy_decision_download (decision);

     return TRUE;
}

static gboolean module_web_view_cb_download (ModuleWebView *webview, WebKitDownload *download, gpointer data)
{
     gboolean ret = FALSE;

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[NEW_DOWNLOAD_SIGNAL],
          0,
          download,
          &ret
     );

     return ret;
}

static void module_web_view_cb_hoverlink (ModuleWebView *webview, gchar *title, gchar *link, gpointer data)
{
     if (webview->status != NULL)
          g_free (webview->status);

     if (link == NULL)
          webview->status = g_strdup (webview->uri);
     else
          webview->status = g_strdup_printf ("Link: %s", link);

     g_signal_emit (
          G_OBJECT (webview),
          module_web_view_signals[STATUS_CHANGED_SIGNAL],
          0,
          webview->status
     );
}

/* ModuleWebView methods */
void module_web_view_load_uri (ModuleWebView *view, const gchar *uri)
{
     webkit_web_view_load_uri (WEBKIT_WEB_VIEW (view), uri);

     view->uri = g_strdup (webkit_web_view_get_uri (WEBKIT_WEB_VIEW (view)));
     view->title = g_strdup (webkit_web_view_get_title (WEBKIT_WEB_VIEW (view)));
     view->ico = favicon_new (webkit_web_view_get_icon_uri (WEBKIT_WEB_VIEW (view)));
}

/* Execute JavaScript script */
static gchar *jsapi_ref_to_string (JSContextRef context, JSValueRef ref)
{
     JSStringRef string_ref;
     gchar *string;
     size_t length;

     string_ref = JSValueToStringCopy (context, ref, NULL);
     length = JSStringGetMaximumUTF8CStringSize (string_ref);
     string = g_new (gchar, length);
     JSStringGetUTF8CString (string_ref, string, length);
     JSStringRelease (string_ref);

     return string;
}

static void jsapi_evaluate_script (ModuleWebView *view, const gchar *script, gchar **value, gchar **message)
{
     WebKitWebFrame *frame = webkit_web_view_get_main_frame (WEBKIT_WEB_VIEW (view));
     JSGlobalContextRef context = webkit_web_frame_get_global_context (frame);
     JSStringRef str;
     JSValueRef val, exception;

     str = JSStringCreateWithUTF8CString (script);
     val = JSEvaluateScript (context, str, JSContextGetGlobalObject (context), NULL, 0, &exception);

     if (!val)
          if (message != NULL)
               *message = jsapi_ref_to_string (context, exception);
     else
          if (value != NULL)
               *value = jsapi_ref_to_string (context, val);
}

void module_web_view_js_script_execute (ModuleWebView *view, const gchar *script)
{
     gchar *value = NULL, *message = NULL;

     g_return_if_fail (script != NULL);

     jsapi_evaluate_script (view, script, &value, &message);

     if (view->jsmsg)
          g_free (view->jsmsg);

     if (message)
     {
          view->jsmsg = g_strdup (message);
          g_free (message);
     }

     if (value)
     {
          view->jsmsg = g_strdup (value);
          g_free (value);
     }
}

const gchar *module_web_view_get_uri (ModuleWebView *view)
{
     return (const gchar *) view->uri;
}

const gchar *module_web_view_get_title (ModuleWebView *view)
{
     return (const gchar *) view->title;
}

const gchar *module_web_view_get_jsmsg (ModuleWebView *view)
{
     return (const gchar *) view->jsmsg;
}

const gchar *module_web_view_get_status (ModuleWebView *view)
{
     return (const gchar *) view->status;
}

gboolean module_web_view_get_view_source_mode (ModuleWebView *view)
{
     return webkit_web_view_get_view_source_mode (WEBKIT_WEB_VIEW (view));
}

void module_web_view_set_view_source_mode (ModuleWebView *view, gboolean mode)
{
     webkit_web_view_set_view_source_mode (WEBKIT_WEB_VIEW (view), mode);
}

gfloat module_web_view_get_zoom_level (ModuleWebView *view)
{
     return webkit_web_view_get_zoom_level (WEBKIT_WEB_VIEW (view));
}

void module_web_view_zoom_in (ModuleWebView *view)
{
     webkit_web_view_zoom_in (WEBKIT_WEB_VIEW (view));
}

void module_web_view_zoom_out (ModuleWebView *view)
{
     webkit_web_view_zoom_out (WEBKIT_WEB_VIEW (view));
}

void module_web_view_set_zoom_level (ModuleWebView *view, gfloat zoom)
{
     webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (view), zoom);
}

WebKitWebInspector *module_web_view_get_web_inspector (ModuleWebView *view)
{
     return view->inspector;
}

WebKitWebSettings *module_web_view_get_settings (ModuleWebView *view)
{
     return view->settings;
}

Favicon *module_web_view_get_favicon (ModuleWebView *view)
{
     return view->ico;
}
