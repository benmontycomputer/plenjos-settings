/* network-settings-window.c
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
#include "network-settings-window.h"

struct _NetworkSettingsWindow
{
  GtkBox  parent_instance;

  GtkStackSwitcher *stack_switcher;

  GtkStack *main_stack;

  NMClient *nm_client;
};

G_DEFINE_TYPE (NetworkSettingsWindow, network_settings_window, GTK_TYPE_BOX)

static void
network_settings_window_class_init (NetworkSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/plenjos/Settings/network-settings/network-settings-window.ui");
  gtk_widget_class_bind_template_child (widget_class, NetworkSettingsWindow, stack_switcher);
  gtk_widget_class_bind_template_child (widget_class, NetworkSettingsWindow, main_stack);
}



static void
network_settings_window_init (NetworkSettingsWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource (cssProvider, "/com/plenjos/Settings/theme.css");
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                               GTK_STYLE_PROVIDER (cssProvider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);

  self->nm_client = nm_client_new (NULL, NULL);

  if (self->nm_client)
		g_print ("NetworkManager version: %s\n", nm_client_get_version (self->nm_client));

  const GPtrArray *devices = nm_client_get_devices (self->nm_client);

  for (size_t i = 0; i < devices->len; i++) {
    printf ("Device: %s\n", nm_device_get_iface (NM_DEVICE (devices->pdata[i])));


  }
  fflush (stdout);

  /*GtkWidget *label = gtk_label_new ("Test 2");
  gtk_widget_set_vexpand (label, TRUE);

  gtk_stack_add_titled (self->main_stack, gtk_list_box_new (), "test", "Test");
  gtk_stack_add_titled (self->main_stack, label, "test2", "Test2");*/
}
