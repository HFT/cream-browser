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

/*!
  \file CreamView.c
  \brief CreamView object
  \author David Delassus

  This object will load an object according to
  the used protocol.
 */

#include <config.h>
#include "CreamView.h"
#include <marshal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void cream_view_class_init (CreamViewClass *class);
static void cream_view_init (CreamView *view);
static void cream_view_load_content (CreamView *view);

static gboolean cream_view_webkit_cb_new_window (ModuleWebView *view,
                                                 WebKitWebFrame *frame,
                                                 WebKitNetworkRequest *request,
                                                 WebKitWebNavigationAction *action,
                                                 WebKitWebPolicyDecision *policy,
                                                 CreamView *obj);
static void cream_view_uri_changed_cb (GtkWidget *w, gchar *uri, CreamView *view);
static void cream_view_new_title_cb (GtkWidget *w, gchar *title, CreamView *view);
static void cream_view_status_changed_cb (GtkWidget *w, gchar *status, CreamView *view);
static gboolean cream_view_new_download_cb (GtkWidget *w, WebKitDownload *download, CreamView *view);
static gboolean cream_view_switch_module_cb (GtkWidget *w, gchar *new_uri, CreamView *view);

/* Signals */

enum
{
     URI_CHANGED_SIGNAL,
     NEW_TITLE_SIGNAL,
     STATUS_CHANGED_SIGNAL,
     NEW_DOWNLOAD_SIGNAL,
     NEW_WINDOW_SIGNAL,
     NB_SIGNALS
};

static guint cream_view_signals[NB_SIGNALS] = { 0 };
/* End of signals */

/*! \struct protocols_t */
struct protocols_t
{
     gchar *prefix;                               /*!< protocol's prefix, for example: http:// */
     void (*func) (CreamView *obj, gchar *uri);   /*!< function to call when the protocol is used */
};

static void cream_view_mailto_callback (CreamView *obj, gchar *uri);
static void cream_view_about_callback (CreamView *obj, gchar *uri);
static void cream_view_http_callback (CreamView *obj, gchar *uri);
static void cream_view_ftp_callback (CreamView *obj, gchar *uri);

/*! \brief Available protocoles and their callbacks */
struct protocols_t cream_available_protocols[] =
{
     { "mailto:",   cream_view_mailto_callback },
     { "about:",    cream_view_about_callback },
     { "file://",   cream_view_http_callback },
     { "http://",   cream_view_http_callback },
     { "https://",  cream_view_http_callback },
     { "ftp://",    cream_view_ftp_callback },
#ifdef HAVE_CURL_SSL
     { "ftps://",   cream_view_ftp_callback }, /*! \todo Manage SSL certification */
#endif
     { "gopher://", NULL },
     { NULL,        NULL }
};

/* End of protocols */

GtkType cream_view_get_type (void)
{
     static GtkType cream_view_type = 0;

     if (!cream_view_type)
     {
          static const GtkTypeInfo cream_view_info =
          {
               "CreamView",
               sizeof (CreamView),
               sizeof (CreamViewClass),
               (GtkClassInitFunc) cream_view_class_init,
               (GtkObjectInitFunc) cream_view_init,
               NULL,
               NULL,
               (GtkClassInitFunc) NULL
          };

          cream_view_type = gtk_type_unique (gtk_scrolled_window_get_type (), &cream_view_info);
     }

     return cream_view_type;
}

static void cream_view_class_init (CreamViewClass *class)
{
     cream_view_signals[URI_CHANGED_SIGNAL] = g_signal_new ("uri-changed",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamViewClass, uri_changed),
               NULL, NULL,
               g_cclosure_marshal_VOID__STRING,
               G_TYPE_NONE,
               1, G_TYPE_STRING);

     cream_view_signals[NEW_TITLE_SIGNAL] = g_signal_new ("new-title",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamViewClass, new_title),
               NULL, NULL,
               g_cclosure_marshal_VOID__STRING,
               G_TYPE_NONE,
               1, G_TYPE_STRING);

     cream_view_signals[STATUS_CHANGED_SIGNAL] = g_signal_new ("status-changed",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamViewClass, status_changed),
               NULL, NULL,
               g_cclosure_marshal_VOID__STRING,
               G_TYPE_NONE,
               1, G_TYPE_STRING);

     cream_view_signals[NEW_WINDOW_SIGNAL] = g_signal_new ("new-window",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamViewClass, new_window),
               NULL, NULL,
               g_cclosure_user_marshal_BOOLEAN__STRING,
               G_TYPE_BOOLEAN,
               1, G_TYPE_STRING);

     cream_view_signals[NEW_DOWNLOAD_SIGNAL] = g_signal_new ("new-download",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamViewClass, new_download),
               NULL, NULL,
               g_cclosure_user_marshal_BOOLEAN__OBJECT,
               G_TYPE_BOOLEAN,
               1, G_TYPE_OBJECT);
}

static void cream_view_init (CreamView *obj)
{
     GtkWidget *scroll_h = gtk_hscrollbar_new (NULL);
     GtkWidget *scroll_v = gtk_vscrollbar_new (NULL);

     obj->adjust_h = gtk_range_get_adjustment (GTK_RANGE (scroll_h));
     obj->adjust_v = gtk_range_get_adjustment (GTK_RANGE (scroll_v));

     gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (obj), obj->adjust_h);
     gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (obj), obj->adjust_v);

     obj->content = NULL;
     obj->uri = NULL;
     obj->title = NULL;
     obj->status = NULL;
     obj->view_source_mode = FALSE;
     obj->history = cream_backward_forward_list_new ();
}

static void cream_view_mailto_callback (CreamView *view, gchar *uri)
{
     if (fork ())
     {
          if (fork ())
          {
               execl ("/bin/sh", "sh", "-c", g_strconcat ("xdg-open ", uri, NULL), (char *) NULL);
               exit (EXIT_SUCCESS);
          }
          exit (EXIT_SUCCESS);
     }
}

static void cream_view_about_callback (CreamView *obj, gchar *uri)
{
     obj->content = module_web_view_new ();
     module_web_view_load_uri (MODULE_WEB_VIEW (obj->content), uri);
}

static void cream_view_http_callback (CreamView *obj, gchar *uri)
{
     obj->content = module_web_view_new ();
     module_web_view_set_view_source_mode (MODULE_WEB_VIEW (obj->content), obj->view_source_mode);
     module_web_view_load_uri (MODULE_WEB_VIEW (obj->content), uri);

     g_signal_connect (G_OBJECT (obj->content),
          "new-window-policy-decision-requested", G_CALLBACK (cream_view_webkit_cb_new_window), obj
     );
}

static void cream_view_ftp_callback (CreamView *obj, gchar *uri)
{
     obj->content = module_ftp_new ();
     module_ftp_load_uri (MODULE_FTP (obj->content), uri);
}

/*!
  \fn static void cream_view_load_content (CreamView *view)
  \brief Load an object according to the used protocol

  \param view The CreamView object
 */
static void cream_view_load_content (CreamView *view)
{
     gchar *uri = (view->uri[0] == '/' ? g_strconcat ("file://", view->uri, NULL) : view->uri);
     gboolean success = FALSE;
     int i;

     for (i = 0; cream_available_protocols[i].prefix != NULL; ++i)
     {
          if (g_str_has_prefix (uri, cream_available_protocols[i].prefix)
               && cream_available_protocols[i].func != NULL)
          {
               cream_available_protocols[i].func (view, uri);
               success = TRUE;
               break;
          }
     }

     if (!success)
          cream_view_http_callback (view, g_strconcat ("http://", uri, NULL));

     g_object_connect (G_OBJECT (view->content),
          "signal::uri-changed",    G_CALLBACK (cream_view_uri_changed_cb),    view,
          "signal::new-title",      G_CALLBACK (cream_view_new_title_cb),      view,
          "signal::status-changed", G_CALLBACK (cream_view_status_changed_cb), view,
          "signal::new-download",   G_CALLBACK (cream_view_new_download_cb),   view,
          "signal::switch-module",  G_CALLBACK (cream_view_switch_module_cb),  view,
     NULL);
}

/*!
  \fn GtkWidget *cream_view_new (void)
  \brief Create a new CreamView object

  \return The new CreamView object
 */
GtkWidget *cream_view_new (void)
{
     CreamView *view = gtk_type_new (cream_view_get_type ());

     g_return_val_if_fail (view != NULL, NULL);

     view->uri = g_strdup ("about:blank");
     cream_view_load_content (view);
     gtk_container_add (GTK_CONTAINER (view), view->content);

     return GTK_WIDGET (view);
}

/* methods */

/*!
  \fn void cream_view_load_uri (CreamView *obj, const gchar *uri)
  \brief Load a new URI

  \param obj The CreamView object
  \param uri The new URI to load
 */
void cream_view_load_uri (CreamView *obj, const gchar *uri)
{
     if (uri != NULL)
          obj->uri = g_strdup (uri);
     else
          obj->uri = g_strdup ("about:blank");

     gtk_container_remove (GTK_CONTAINER (obj), obj->content);
     cream_view_load_content (obj);
     gtk_container_add (GTK_CONTAINER (obj), obj->content);
     gtk_widget_show_all (GTK_WIDGET (obj));
}

/*!
  \fn GtkWidget *cream_view_get_content (CreamView *obj)
  \brief Get the object which show the URI previously loaded

  \param obj The CreamView object
  \return The CreamView's content
 */
GtkWidget *cream_view_get_content (CreamView *obj)
{
     return obj->content;
}

/*!
  \fn gboolean cream_view_get_view_source_mode (CreamView *obj)
  \brief Check if we are seeing the page's source
 */
gboolean cream_view_get_view_source_mode (CreamView *obj)
{
     return obj->view_source_mode;
}

/*!
  \fn void cream_view_set_view_source_mode (CreamView *obj, gboolean mode)
  \brief Tell the CreamView to show the page's source or not

  \param obj
  \param mode TRUE if you want show the page's source
 */
void cream_view_set_view_source_mode (CreamView *obj, gboolean mode)
{
     obj->view_source_mode = mode;
}

/*! \fn gboolean cream_view_can_go_back (CreamView *obj) */
gboolean cream_view_can_go_back (CreamView *obj)
{
     return FALSE; /* TODO */
}

/*! \fn gboolean cream_view_can_go_forward (CreamView *obj) */
gboolean cream_view_can_go_forward (CreamView *obj)
{
     return FALSE; /* TODO */
}

/*! \fn void cream_view_go_back (CreamView *obj) */
void cream_view_go_back (CreamView *obj)
{
     /* TODO */;
}

/*! \fn void cream_view_go_forward (CreamView *obj) */
void cream_view_go_forward (CreamView *obj)
{
     /* TODO */;
}

/*! \fn CreamBackwardForwardList *cream_view_get_backward_forward_list (CreamView *obj) */
CreamBackwardForwardList *cream_view_get_backward_forward_list (CreamView *obj)
{
     return obj->history;
}

/*! \fn GtkAdjustment *cream_view_get_hadjustment (CreamView *obj) */
GtkAdjustment *cream_view_get_hadjustment (CreamView *obj)
{
     return obj->adjust_h;
}

/*! \fn GtkAdjustment *cream_view_get_vadjustment (CreamView *obj) */
GtkAdjustment *cream_view_get_vadjustment (CreamView *obj)
{
     return obj->adjust_v;
}

/*!
  \fn const gchar *cream_view_get_uri (CreamView *obj)
  \brief Get the URI of the loaded page
 */
const gchar *cream_view_get_uri (CreamView *obj)
{
     return (const gchar *) obj->uri;
}

/*!
  \fn const gchar *cream_view_get_title (CreamView *obj)
  \brief Get the title of the loaded page
 */
const gchar *cream_view_get_title (CreamView *obj)
{
     return (const gchar *) obj->title;
}

/*!
  \fn const gchar *cream_view_get_status (CreamView *obj)
  \brief Get the status message of the loaded page (you can show it in a statusbar)
 */
const gchar *cream_view_get_status (CreamView *obj)
{
     return (const gchar *) obj->status;
}

/*!
  \fn GdkPixbuf *cream_view_get_favicon (CreamView *obj)
  \brief Get the Favicon of the loaded page
  \return A pixbuf containing the favicon image
 */
GdkPixbuf *cream_view_get_favicon (CreamView *obj)
{
     if (MODULE_IS_WEB_VIEW (obj->content))
     {
          Favicon *tmp = module_web_view_get_favicon (MODULE_WEB_VIEW (obj->content));
          if (tmp) return tmp->ico;
     }

     return NULL;
}

/* signals */
static gboolean cream_view_webkit_cb_new_window (ModuleWebView *view,
                                                 WebKitWebFrame *frame,
                                                 WebKitNetworkRequest *request,
                                                 WebKitWebNavigationAction *action,
                                                 WebKitWebPolicyDecision *policy,
                                                 CreamView *obj)
{
     gboolean ret = FALSE;
     const gchar *uri = webkit_network_request_get_uri (request);

     g_signal_emit (
          G_OBJECT (obj),
          cream_view_signals[NEW_WINDOW_SIGNAL],
          0, uri, &ret
     );

     if (ret)
          webkit_web_policy_decision_ignore (policy);
     return ret;
}

static void cream_view_uri_changed_cb (GtkWidget *w, gchar *uri, CreamView *view)
{
     if (view->uri)
          g_free (view->uri);
     view->uri = g_strdup (uri);

     g_signal_emit (
          G_OBJECT (view),
          cream_view_signals[URI_CHANGED_SIGNAL],
          0, uri
     );
}

static void cream_view_new_title_cb (GtkWidget *w, gchar *title, CreamView *view)
{
     if (view->title)
          g_free (view->title);
     view->title = g_strdup (title);

     g_signal_emit (
          G_OBJECT (view),
          cream_view_signals[NEW_TITLE_SIGNAL],
          0, title
     );
}

static void cream_view_status_changed_cb (GtkWidget *w, gchar *status, CreamView *view)
{
     if (view->status)
          g_free (view->status);
     view->status = g_strdup (status);

     g_signal_emit (
          G_OBJECT (view),
          cream_view_signals[STATUS_CHANGED_SIGNAL],
          0, status
     );
}

static gboolean cream_view_new_download_cb (GtkWidget *w, WebKitDownload *download, CreamView *view)
{
     gboolean ret = FALSE;

     g_signal_emit (
          G_OBJECT (view),
          cream_view_signals[NEW_DOWNLOAD_SIGNAL],
          0, download,
          &ret
     );

     return ret;
}

static gboolean cream_view_switch_module_cb (GtkWidget *w, gchar *new_uri, CreamView *view)
{
     cream_view_load_uri (view, new_uri);
     return TRUE;
}
