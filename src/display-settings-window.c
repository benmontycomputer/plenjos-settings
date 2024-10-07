/* display-settings-window.c
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
#include "display-settings-window.h"

struct _DisplaySettingsWindow
{
  GtkBox  parent_instance;

  GtkBox *displays_box;

  GSList *displays_list;
};

G_DEFINE_TYPE (DisplaySettingsWindow, display_settings_window, GTK_TYPE_BOX)

static void
display_settings_window_class_init (DisplaySettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/plenjos/Settings/display-settings-window.ui");
  gtk_widget_class_bind_template_child (widget_class, DisplaySettingsWindow, displays_box);
}

/* GtkWidget *add_display_widget (GdkDisplay *display, DisplaySettingsWindow *self) {
  GtkRadioButton *button = GTK_RADIO_BUTTON (gtk_radio_button_new (self->displays_list));

  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);

  self->displays_list = g_slist_append (self->displays_list, button);

  GtkFixed *widget = GTK_FIXED (gtk_fixed_new ());

  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (button)), "display_settings_display");

  GtkLabel *label = GTK_LABEL (gtk_label_new (gdk_display_get_name (display)));

  gtk_fixed_put (widget, label, 20, 20);

  gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (widget));
  gtk_container_add (GTK_CONTAINER (self->displays_box), GTK_WIDGET (button));
} */

static void
display_settings_window_init (DisplaySettingsWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource (cssProvider, "/com/plenjos/Settings/theme.css");
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                               GTK_STYLE_PROVIDER (cssProvider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);
}
