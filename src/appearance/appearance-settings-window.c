/* appearance-settings-window.c
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

#include "settings-config.h"
#include "appearance-settings-window.h"

struct _AppearanceSettingsWindow
{
  AdwNavigationPage parent_instance;

  AdwActionRow *bg_selector;

  GtkFileDialog *file_dialog;
  GSettings *bg_settings;
  GSettings *interface_settings;
  GtkPicture *bg_picture;

  AdwPreferencesPage *appearance_settings_preferences_page;
};

G_DEFINE_TYPE(AppearanceSettingsWindow, appearance_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
appearance_settings_window_class_init(AppearanceSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/appearance/appearance-settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, appearance_settings_preferences_page);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, bg_selector);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, bg_picture);
}

static void on_bg_selector_ready(GObject *source_object, GAsyncResult *res, AppearanceSettingsWindow *self)
{
  GFile *result = gtk_file_dialog_open_finish(self->file_dialog, res, NULL);

  char *path = g_file_get_path(result);

  printf("%s\n", path);
  fflush(stdout);

  size_t path_uri_len = strlen(path) + strlen("file://") + 1;
  char *path_uri = malloc(path_uri_len);
  snprintf(path_uri, path_uri_len, "file://%s", path);

  g_settings_set_string(self->bg_settings, "picture-uri", path_uri);
  g_settings_set_string(self->bg_settings, "picture-uri-dark", path_uri);

  free(path_uri);
}

static void on_bg_selector_activated(AdwActionRow *bg_selector, AppearanceSettingsWindow *self)
{
  GtkWidget *win = GTK_WIDGET(&self->parent_instance);

  while (win && !GTK_IS_WINDOW(win))
  {
    win = gtk_widget_get_parent(win);
  }

  gtk_file_dialog_open(self->file_dialog, GTK_WINDOW(win), NULL, (GAsyncReadyCallback)on_bg_selector_ready, self);
}

static void
appearance_settings_window_init(AppearanceSettingsWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  self->file_dialog = gtk_file_dialog_new();

  self->bg_settings = g_settings_new("org.gnome.desktop.background");
  self->interface_settings = g_settings_new("org.gnome.desktop.interface");

  char *scheme = g_settings_get_string(self->interface_settings, "color-scheme");

  if (!strcmp(scheme, "prefer-dark"))
  {
    char *bg = g_settings_get_string(self->bg_settings, "picture-uri-dark");
    gtk_picture_set_filename(self->bg_picture, (char *)(bg + 7));
    free(bg);
  }
  else
  {
    char *bg = g_settings_get_string(self->bg_settings, "picture-uri");
    gtk_picture_set_filename(self->bg_picture, (char *)(bg + 7));
    free(bg);
  }

  if (scheme)
  {
    free(scheme);
  }

  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(self->bg_selector), TRUE);
  g_signal_connect(self->bg_selector, "activated", G_CALLBACK(on_bg_selector_activated), self);
}
