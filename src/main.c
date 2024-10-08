/* main.c
 *
 * Copyright 2023 Benjamin Montgomery
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "settings-config.h"
#include "settings-window.h"

static void
on_activate(GtkApplication *app)
{
	GtkWindow *window;

	/* It's good practice to check your parameters at the beginning of the
	 * function. It helps catch errors early and in development instead of
	 * by your users.
	 */
	g_assert(GTK_IS_APPLICATION(app));

	/* Get the current window or create one if necessary. */
	window = gtk_application_get_active_window(app);
	if (window == NULL)
		window = g_object_new(SETTINGS_TYPE_WINDOW,
							  "application", app,
							  NULL);

	/* GdkPixbuf *icon_24 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 24, 0, NULL);
	GdkPixbuf *icon_32 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 32, 0, NULL);
	GdkPixbuf *icon_48 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 48, 0, NULL);
	GdkPixbuf *icon_64 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 64, 0, NULL);
	GdkPixbuf *icon_96 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 96, 0, NULL);
	GdkPixbuf *icon_128 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 128, 0, NULL);
	GdkPixbuf *icon_256 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "systemsettings", 256, 0, NULL);

	GList *icons = NULL;

	icons = g_list_append (icons, icon_24);
	icons = g_list_append (icons, icon_32);
	icons = g_list_append (icons, icon_48);
	icons = g_list_append (icons, icon_64);
	icons = g_list_append (icons, icon_96);
	icons = g_list_append (icons, icon_128);
	icons = g_list_append (icons, icon_256);

	gtk_window_set_icon_list (window, icons);

	g_list_free (icons);

	g_object_unref (icon_24);
	g_object_unref (icon_32);
	g_object_unref (icon_48);
	g_object_unref (icon_64);
	g_object_unref (icon_96);
	g_object_unref (icon_128);
	g_object_unref (icon_256); */

	/* Ask the window manager/compositor to present the window. */
	gtk_window_present(window);
}

int main(int argc,
		 char *argv[])
{
	g_autoptr(GtkApplication) app = NULL;
	int ret;

	/* Set up gettext translations */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	/*
	 * Create a new GtkApplication. The application manages our main loop,
	 * application windows, integration with the window manager/compositor, and
	 * desktop features such as file opening and single-instance applications.
	 */
	app = gtk_application_new("com.plenjos.Settings", G_APPLICATION_DEFAULT_FLAGS);

	/*
	 * We connect to the activate signal to create a window when the application
	 * has been launched. Additionally, this signal notifies us when the user
	 * tries to launch a "second instance" of the application. When they try
	 * to do that, we'll just present any existing window.
	 *
	 * Because we can't pass a pointer to any function type, we have to cast
	 * our "on_activate" function to a GCallback.
	 */
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

	/*
	 * Run the application. This function will block until the application
	 * exits. Upon return, we have our exit code to return to the shell. (This
	 * is the code you see when you do `echo $?` after running a command in a
	 * terminal.
	 *
	 * Since GtkApplication inherits from GApplication, we use the parent class
	 * method "run". But we need to cast, which is what the "G_APPLICATION()"
	 * macro does.
	 */
	ret = g_application_run(G_APPLICATION(app), argc, argv);

	return ret;
}
