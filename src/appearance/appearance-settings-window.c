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

  AdwPreferencesPage *appearance_settings_preferences_page;
};

G_DEFINE_TYPE(AppearanceSettingsWindow, appearance_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
appearance_settings_window_class_init(AppearanceSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/appearance/appearance-settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, AppearanceSettingsWindow, appearance_settings_preferences_page);
}

static void
appearance_settings_window_init(AppearanceSettingsWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));
}