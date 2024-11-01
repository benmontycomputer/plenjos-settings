/* panel-settings-window.c
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
#include "panel-settings-window.h"

struct _PanelSettingsWindow
{
  AdwNavigationPage parent_instance;

  GSettings *panel_settings;

  AdwComboRow *panel_style_combo_row;
  AdwPreferencesPage *panel_settings_preferences_page;
};

G_DEFINE_TYPE(PanelSettingsWindow, panel_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
panel_settings_window_class_init(PanelSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/panel/panel-settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, PanelSettingsWindow, panel_settings_preferences_page);
  gtk_widget_class_bind_template_child(widget_class, PanelSettingsWindow, panel_style_combo_row);
}

static void on_panel_style_selected(AdwComboRow *row, gpointer *idk, PanelSettingsWindow *self) {
  guint item = adw_combo_row_get_selected(row);

  if (item == 1) {
    g_settings_set_string(self->panel_settings, "color-scheme", "prefer-light");
  } else if (item == 2) {
    g_settings_set_string(self->panel_settings, "color-scheme", "prefer-dark");
  } else {
    g_settings_set_string(self->panel_settings, "color-scheme", "default");
  }
}

static void panel_settings_window_init(PanelSettingsWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  self->panel_settings = g_settings_new("com.plenjos.shell.panel");

  g_signal_connect(self->panel_style_combo_row, "notify::selected", G_CALLBACK(on_panel_style_selected), self);
}
