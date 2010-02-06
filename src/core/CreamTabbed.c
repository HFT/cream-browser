#include "CreamTabbed.h"

enum
{
     UPDATE_NOTEBOOK_TITLE_SIGNAL,
     NB_SIGNALS
};

static guint cream_tabbed_signals[NB_SIGNALS] = { 0 };

static void cream_tabbed_class_init (CreamTabbedClass *class);
static void cream_tabbed_init (CreamTabbed *obj);

static void cream_tabbed_init_statusbar (CreamTabbed *obj);
static void cream_tabbed_init_inputbox (CreamTabbed *obj);

static void cream_tabbed_uri_changed_cb (CreamTabbed *obj, gchar *uri, gpointer data);
static void cream_tabbed_new_title_cb (CreamTabbed *obj, gchar *title, gpointer data);
static void cream_tabbed_status_changed_cb (CreamTabbed *obj, gchar *status, gpointer data);
static void cream_tabbed_scroll_cb (GtkAdjustment *vadjust, gpointer data);

GtkType cream_tabbed_get_type (void)
{
     static GtkType cream_tabbed_type = 0;

     if (!cream_tabbed_type)
     {
          static const GtkTypeInfo cream_tabbed_info =
          {
               "CreamTabbed",
               sizeof (CreamTabbed),
               sizeof (CreamTabbedClass),
               (GtkClassInitFunc) cream_tabbed_class_init,
               (GtkObjectInitFunc) cream_tabbed_init,
               NULL, NULL,
               (GtkClassInitFunc) NULL
          };

          cream_tabbed_type = gtk_type_unique (gtk_vbox_get_type (), &cream_tabbed_info);
     }

     return cream_tabbed_type;
}

static void cream_tabbed_class_init (CreamTabbedClass *class)
{
     cream_tabbed_signals[UPDATE_NOTEBOOK_TITLE_SIGNAL] = g_signal_new ("update-notebook-title",
               G_TYPE_FROM_CLASS (class),
               G_SIGNAL_RUN_LAST,
               G_STRUCT_OFFSET (CreamTabbedClass, update_notebook_title),
               NULL, NULL,
               g_cclosure_marshal_VOID__VOID,
               G_TYPE_NONE,
               0, G_TYPE_NONE);
}

static void cream_tabbed_init (CreamTabbed *obj)
{
     obj->creamview = NULL;

     obj->statusbar.eventbox = NULL;
     obj->statusbar.box      = NULL;
     obj->statusbar.url      = NULL;
     obj->statusbar.state    = NULL;

     obj->inputbox = NULL;
     obj->adjust_v = NULL;
}

GtkWidget *cream_tabbed_new (void)
{
     CreamTabbed *obj = gtk_type_new (cream_tabbed_get_type ());

     obj->creamview = cream_view_new ();
     obj->adjust_v = cream_view_get_vadjustment (CREAM_VIEW (obj->creamview));

     cream_tabbed_init_statusbar (obj);
     cream_tabbed_init_inputbox (obj);

     gtk_box_pack_end (GTK_BOX (obj), obj->inputbox, FALSE, FALSE, 0);
     gtk_box_pack_end (GTK_BOX (obj), obj->statusbar.eventbox, FALSE, FALSE, 0);
     gtk_box_pack_start (GTK_BOX (obj), obj->creamview, TRUE, TRUE, 0);

     g_signal_connect (G_OBJECT (obj->adjust_v),  "value-changed",  G_CALLBACK (cream_tabbed_scroll_cb),         obj);
     g_signal_connect (G_OBJECT (obj->creamview), "uri-changed",    G_CALLBACK (cream_tabbed_uri_changed_cb),    NULL);
     g_signal_connect (G_OBJECT (obj->creamview), "new-title",      G_CALLBACK (cream_tabbed_new_title_cb),      NULL);
     g_signal_connect (G_OBJECT (obj->creamview), "status-changed", G_CALLBACK (cream_tabbed_status_changed_cb), NULL);

     return GTK_WIDGET (obj);
}

static void cream_tabbed_init_statusbar (CreamTabbed *obj)
{
     GdkColor bg, fg;

     obj->statusbar.eventbox = gtk_event_box_new ();
     obj->statusbar.box      = gtk_hbox_new (FALSE, 0);
     obj->statusbar.url      = gtk_label_new (NULL);
     obj->statusbar.state    = gtk_label_new (NULL);

     /* change statusbar color */
     gdk_color_parse ("#000000", &bg);
     gtk_widget_modify_bg (obj->statusbar.eventbox, GTK_STATE_NORMAL, &bg);

     gdk_color_parse ("#FFFFFF", &fg);
     gtk_widget_modify_fg (obj->statusbar.url, GTK_STATE_NORMAL, &fg);
     gtk_widget_modify_fg (obj->statusbar.state, GTK_STATE_NORMAL, &fg);

     /* align the URL and the state */
     gtk_misc_set_alignment (GTK_MISC (obj->statusbar.url), 0.0, 0.0);
     gtk_misc_set_alignment (GTK_MISC (obj->statusbar.state), 1.0, 0.0);

     gtk_label_set_selectable (GTK_LABEL (obj->statusbar.url), TRUE);

     gtk_box_pack_start (GTK_BOX (obj->statusbar.box), obj->statusbar.url, TRUE, TRUE, 2);
     gtk_box_pack_start (GTK_BOX (obj->statusbar.box), obj->statusbar.state, FALSE, FALSE, 2);

     gtk_container_add (GTK_CONTAINER (obj->statusbar.eventbox), obj->statusbar.box);
}

static void cream_tabbed_init_inputbox (CreamTabbed *obj)
{
     PangoFontDescription *font;

     obj->inputbox = gtk_entry_new ();

     /* Change inputbox style */
     /* font */
     font = pango_font_description_from_string ("monospace normal 8");
     gtk_widget_modify_font (obj->inputbox, font);
     pango_font_description_free (font);

     /* border */
     gtk_entry_set_inner_border (GTK_ENTRY (obj->inputbox), NULL);
     gtk_entry_set_has_frame (GTK_ENTRY (obj->inputbox), FALSE);
}

void cream_tabbed_load_uri (CreamTabbed *obj, gchar *uri)
{
     cream_view_load_uri (CREAM_VIEW (obj->creamview), uri);
}

const gchar *cream_tabbed_get_uri (CreamTabbed *obj)
{
     return cream_view_get_uri (CREAM_VIEW (obj->creamview));
}

const gchar *cream_tabbed_get_title (CreamTabbed *obj)
{
     return cream_view_get_title (CREAM_VIEW (obj->creamview));
}

const gchar *cream_tabbed_get_status (CreamTabbed *obj)
{
     return cream_view_get_status (CREAM_VIEW (obj->creamview));
}

/* signals */
static void cream_tabbed_uri_changed_cb (CreamTabbed *obj, gchar *uri, gpointer data)
{
     GdkColor color;
     gboolean ssl = g_str_has_prefix (uri, "https://");
     gboolean back = cream_view_can_go_back (CREAM_VIEW (obj->creamview));
     gboolean fwd = cream_view_can_go_forward (CREAM_VIEW (obj->creamview));

     if (back || fwd)
     {
          gtk_label_set_markup (GTK_LABEL (obj->statusbar.url), g_markup_printf_escaped (
               "<span font=\"monospace bold 8\">%s</span> [%s%s]", uri, (back ? "+" : ""), (fwd ? "-" : ""))
          );
     }
     else
     {
          gtk_label_set_markup (GTK_LABEL (obj->statusbar.url), g_markup_printf_escaped (
               "<span font=\"monospace bold 8\">%s</span>", uri)
          );
     }

     gdk_color_parse ((ssl ? "#B0FF00" : "#000000"), &color);
     gtk_widget_modify_bg (obj->statusbar.eventbox, GTK_STATE_NORMAL, &color);

     gdk_color_parse ((ssl ? "#000000" : "#FFFFFF"), &color);
     gtk_widget_modify_fg (obj->statusbar.url, GTK_STATE_NORMAL, &color);
     gtk_widget_modify_fg (obj->statusbar.state, GTK_STATE_NORMAL, &color);

     g_signal_emit (
          G_OBJECT (obj),
          cream_tabbed_signals[UPDATE_NOTEBOOK_TITLE_SIGNAL],
          0
     );
}

static void cream_tabbed_new_title_cb (CreamTabbed *obj, gchar *title, gpointer data)
{
     g_signal_emit (
          G_OBJECT (obj),
          cream_tabbed_signals[UPDATE_NOTEBOOK_TITLE_SIGNAL],
          0
     );
}

static void cream_tabbed_status_changed_cb (CreamTabbed *obj, gchar *status, gpointer data)
{
     gtk_label_set_markup (GTK_LABEL (obj->statusbar.url), g_markup_printf_escaped (
          "<span font=\"monospace bold 8\">%s</span>", status)
     );
}

static void cream_tabbed_scroll_cb (GtkAdjustment *vadjust, gpointer data)
{
     CreamTabbed *obj = (CreamTabbed *) data;
     gint max = gtk_adjustment_get_upper (vadjust) - gtk_adjustment_get_page_size (vadjust);
     gint val = (gint) (gtk_adjustment_get_value (vadjust) / max * 100);
     gchar *str;

     if (max == 0) /* If we see all the page */
          str = g_strdup ("All");
     else if (val == 0) /* we are on the page's top */
          str = g_strdup ("Top");
     else if (val == 100) /* we are on the page's bottom */
          str = g_strdup ("Bot");
     else
          str = g_strdup_printf ("%d%%", val);

     gtk_label_set_markup (GTK_LABEL (obj->statusbar.state), g_markup_printf_escaped ("<span font=\"monospace bold 8\">%s</span>", str));
}
