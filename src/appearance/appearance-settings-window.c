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

  AdwComboRow *theme_combo_row;
  AdwActionRow *bg_selector;

  GtkFileDialog *file_dialog;
  GSettings *bg_settings;
  GSettings *interface_settings;
  GtkPicture *bg_picture;
  GtkFlowBox *bg_flow_box;

  AdwPreferencesPage *appearance_settings_preferences_page;
};

G_DEFINE_TYPE(AppearanceSettingsWindow, appearance_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
appearance_settings_window_class_init(AppearanceSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/appearance/appearance-settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, appearance_settings_preferences_page);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, theme_combo_row);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, bg_selector);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, bg_picture);
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, bg_flow_box);
}

static void on_bg_selector_ready(GObject *source_object, GAsyncResult *res, AppearanceSettingsWindow *self)
{
  GFile *result = gtk_file_dialog_open_finish(self->file_dialog, res, NULL);

  g_return_if_fail(result);

  char *path = g_file_get_path(result);

  printf("%s\n", path);
  fflush(stdout);

  g_settings_set_string(self->bg_settings, "background", path);

  free(path);
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

static void on_theme_selected(AdwComboRow *row, gpointer *idk, AppearanceSettingsWindow *self)
{
  guint item = adw_combo_row_get_selected(row);

  if (item == 1)
  {
    g_settings_set_string(self->interface_settings, "color-scheme", "prefer-light");
  }
  else if (item == 2)
  {
    g_settings_set_string(self->interface_settings, "color-scheme", "prefer-dark");
  }
  else
  {
    g_settings_set_string(self->interface_settings, "color-scheme", "default");
  }
}

static void
update_bg(GSettings *bg_settings, gchar *key, AppearanceSettingsWindow *self)
{
  char *bg = g_settings_get_string(bg_settings, key);

  // gtk_image_set_from_file(self->bg_image, bg);
  gtk_picture_set_filename(self->bg_picture, bg);

  free(bg);
}

static void appearance_settings_window_init(AppearanceSettingsWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  self->file_dialog = gtk_file_dialog_new();

  self->bg_settings = g_settings_new("com.plenjos.shell.desktop");
  self->interface_settings = g_settings_new("org.gnome.desktop.interface");

  char *scheme = g_settings_get_string(self->interface_settings, "color-scheme");

  if (!strcmp(scheme, "prefer-dark"))
  {
    adw_combo_row_set_selected(self->theme_combo_row, 2);
  }
  else
  {
    if (!strcmp(scheme, "prefer-light"))
    {
      adw_combo_row_set_selected(self->theme_combo_row, 1);
    }
    else
    {
      adw_combo_row_set_selected(self->theme_combo_row, 0);
    }
  }

  if (scheme)
  {
    free(scheme);
  }

  g_signal_connect(self->bg_settings, "changed::background",
                   G_CALLBACK(update_bg), self);

  update_bg(self->bg_settings, "background", self);

  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(self->bg_selector), TRUE);
  g_signal_connect(self->bg_selector, "activated", G_CALLBACK(on_bg_selector_activated), self);

  g_signal_connect(self->theme_combo_row, "notify::selected", G_CALLBACK(on_theme_selected), self);

  /*GTask *taskbar_task = g_task_new(self, NULL, NULL, NULL);
  g_task_set_task_data(taskbar_task, self, NULL);
  g_task_run_in_thread(taskbar_task, (GTaskThreadFunc)load_backgrounds_wrap);*/
  // load_backgrounds("/usr/share/backgrounds", self, true);
}
