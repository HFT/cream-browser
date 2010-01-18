#include "local.h"

struct global_t global;

GOptionEntry entries[] =
{
     { "config", 'c', 0, G_OPTION_ARG_FILENAME, &global.cmdline.config, "Load an alternative configuration file" },
     { "open", 'o', 0, G_OPTION_ARG_STRING, &global.cmdline.url, "Open URL") },
     { NULL }
};

gboolean cream_init (int *argv, char ***argv, GError **error)
{
     GError *local_error = NULL;
     GOptionContext *ctx;
     const gchar *home;

     /* init global structure */
     global.cmdline.config = NULL;
     global.cmdline.url = NULL;

     global.browser.homepage = NULL;
     global.browser.encoding = NULL;

     /* set locale */
     setlocale (LC_ALL, "");
     bindtextdomain (PACKAGE, LOCALEDIR);
     textdomain (PACKAGE);

     signal (SIGSEGV, signal_handler);

     /* parse command line */
     ctx = g_option_context_new ("");
     g_option_context_add_main_entries (ctx, entries, PACKAGE);
     g_option_context_add_group (ctx, gtk_get_option_group (TRUE));

     if (!g_option_context_parse (ctx, argc, argv, &local_error) && local_error != NULL)
     {
          g_propagate_error (error, local_error);
          return FALSE;
     }

     /* crate directories */
     home = g_get_home_dir ();
     g_mkdir_with_parents (g_build_filename (home, ".cream-browser", NULL), 0711);
     g_mkdir_with_parents (g_build_filename (home, ".cream-browser", "downloads", NULL), 0755);

     /* init GTK/Glib */
     gtk_init (argc, argv);

     if (!g_thread_supported ())
          g_thread_init (NULL);

     return TRUE;
}

void cream_release (int exit_code)
{
     soup_cookie_jar_save (global.browser.cookies);

     /* free memory */
     free (global.cmdline.config);
     free (global.cmdline.url);

     free (global.browser.homepage);
     free (global.browser.encoding);

     gtk_main_quit ();
     exit (exit_code);
}

int main (int argc, char **argv)
{
     GError *error = NULL;
     GtkWidget *win;

     if (!cream_init (&argc, &argv, &error) || error != NULL)
     {
          fprintf (stderr, "Error: %s\n", error->message);
          g_error_free (error);
          exit (EXIT_FAILURE);
     }

     win = cream_interface_init ();

     gtk_widget_show_all (win);
     gtk_main ();
     return 0;
}
