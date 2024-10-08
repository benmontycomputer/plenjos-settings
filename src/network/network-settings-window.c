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
  AdwNavigationPage parent_instance;

  AdwPreferencesGroup *interfaces_group;

  NMClient *nm_client;
};

G_DEFINE_TYPE (NetworkSettingsWindow, network_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
network_settings_window_class_init (NetworkSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/plenjos/Settings/network/network-settings-window.ui");
  gtk_widget_class_bind_template_child (widget_class, NetworkSettingsWindow, interfaces_group);
}

typedef struct NetworkSettingsInterface {
  AdwPreferencesRow *iface_row;
  AdwDialog *iface_dialog;
  AdwHeaderBar *header_bar;
  GtkBox *navpage_box;

  char *title;

  NetworkSettingsWindow *self;
} NetworkSettingsInterface;

static void on_iface_activated (AdwActionRow *row, NetworkSettingsInterface *iface) {
  adw_dialog_present (iface->iface_dialog, GTK_WIDGET (row));
}

static GtkWidget *create_net_interface (NMDevice *device, NetworkSettingsWindow *self) {
  const char *description = nm_device_get_description (device);
  const char *name = nm_device_get_iface (device);

  NetworkSettingsInterface *iface = malloc (sizeof (NetworkSettingsInterface));

  iface->self = self;
  iface->iface_row = NULL;
  iface->iface_dialog = NULL;
  iface->header_bar = NULL;
  iface->navpage_box = NULL;
  iface->title = NULL;
  iface->self = NULL;

  iface->self = self;

  size_t title_len = strlen (description) + strlen (name) + strlen (" ()") + 1;
  iface->title = malloc (title_len);
  snprintf (iface->title, title_len, "%s (%s)", description, name);

  iface->navpage_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  iface->header_bar = ADW_HEADER_BAR (adw_header_bar_new ());
  adw_header_bar_set_show_back_button (iface->header_bar, TRUE);
  gtk_box_append (iface->navpage_box, GTK_WIDGET (iface->header_bar));

  iface->iface_dialog = adw_dialog_new ();
  adw_dialog_set_child (iface->iface_dialog, GTK_WIDGET (iface->navpage_box));
  adw_dialog_set_presentation_mode (iface->iface_dialog, ADW_DIALOG_FLOATING);
  adw_dialog_set_title (iface->iface_dialog, iface->title);

  iface->iface_row = ADW_PREFERENCES_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (iface->iface_row, iface->title);
  // adw_action_row_set_activatable_widget (ADW_ACTION_ROW (iface->iface_row), GTK_WIDGET (iface->iface_dialog));
  g_signal_connect (iface->iface_row, "activated", G_CALLBACK (on_iface_activated), iface);

  return GTK_WIDGET (iface->iface_row);
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

    adw_preferences_group_add (self->interfaces_group, create_net_interface (NM_DEVICE (devices->pdata[i]), self));
  }
  fflush (stdout);
}
