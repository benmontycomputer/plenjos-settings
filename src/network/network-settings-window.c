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

  AdwNavigationView *interfaces_view;

  NMClient *nm_client;
};

G_DEFINE_TYPE(NetworkSettingsWindow, network_settings_window, ADW_TYPE_NAVIGATION_PAGE)

static void
network_settings_window_class_init(NetworkSettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/network/network-settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, NetworkSettingsWindow, interfaces_group);
  gtk_widget_class_bind_template_child(widget_class, NetworkSettingsWindow, interfaces_view);
}

typedef struct NetworkSettingsInterface
{
  AdwPreferencesRow *iface_row;
  AdwNavigationPage *iface_page;
  NMDevice *device;
  NMDeviceType device_type;

  char *title;

  NetworkSettingsWindow *self;

  /* Wi-Fi */
  GPtrArray *aps;
  GtkBuilder *wifi_builder;
  AdwPreferencesGroup *wifi_group;
} NetworkSettingsInterface;

static void on_iface_activated(AdwActionRow *row, NetworkSettingsInterface *iface)
{
  adw_navigation_view_push(iface->self->interfaces_view, iface->iface_page);
  printf("test\n");
  fflush(stdout);
}

static void on_wifi_activated(AdwActionRow *row, NMAccessPoint *ap)
{
}

static void aps_foreach(gpointer ap_ptr, NetworkSettingsInterface *iface)
{
  NMAccessPoint *ap = NM_ACCESS_POINT(ap_ptr);

  AdwActionRow *row = ADW_ACTION_ROW(adw_action_row_new());
  GBytes *ssid_bytes = nm_access_point_get_ssid(ap);
  char *ssid_str = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid_bytes, NULL), g_bytes_get_size(ssid_bytes));
  g_bytes_unref(ssid_bytes);
  adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), ssid_str);
  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), TRUE);
  adw_action_row_add_suffix(ADW_ACTION_ROW(row), GTK_WIDGET(gtk_image_new_from_icon_name("go-next")));
  g_signal_connect(row, "activated", G_CALLBACK(on_wifi_activated), ap);

  // adw_action_row_add_suffix(row, nma_wifi_dialog_new_for_create(iface->self->nm_client));
  GtkWidget *win = GTK_WIDGET(&iface->self->parent_instance);
  while (!GTK_IS_WINDOW(win) && GTK_IS_WIDGET(win))
  {
    win = gtk_widget_get_parent(win);
  }
  nma_mobile_wizard_new(GTK_WINDOW (win), NULL, NM_DEVICE_MODEM_CAPABILITY_NONE, TRUE, NULL, NULL);

  adw_preferences_group_add(iface->wifi_group, GTK_WIDGET(row));
}

static GtkWidget *create_net_interface(NMDevice *device, NetworkSettingsWindow *self)
{
  const char *description = nm_device_get_description(device);
  const char *name = nm_device_get_iface(device);

  NetworkSettingsInterface *iface = malloc(sizeof(NetworkSettingsInterface));

  iface->self = self;
  iface->iface_row = NULL;
  iface->iface_page = NULL;
  iface->device = NULL;
  iface->device_type = NM_DEVICE_TYPE_UNKNOWN;
  iface->title = NULL;
  iface->self = NULL;

  iface->device = device;
  iface->self = self;

  size_t title_len = strlen(description) + strlen(name) + strlen(" ()") + 1;
  iface->title = malloc(title_len);
  snprintf(iface->title, title_len, "%s (%s)", description, name);

  iface->device_type = nm_device_get_device_type(device);

  switch (iface->device_type)
  {
  case NM_DEVICE_TYPE_WIFI:
    /* code */
    iface->wifi_builder = gtk_builder_new_from_resource("/com/plenjos/Settings/network/wifi-settings-window.ui");

    iface->iface_page = ADW_NAVIGATION_PAGE(gtk_builder_get_object(iface->wifi_builder, "nav_page"));
    iface->wifi_group = ADW_PREFERENCES_GROUP(gtk_builder_get_object(iface->wifi_builder, "networks_group"));

    iface->aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));

    g_ptr_array_foreach(iface->aps, (GFunc)aps_foreach, iface);
    break;
  default:
    GtkBox *navpage_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    AdwHeaderBar *header_bar = ADW_HEADER_BAR(adw_header_bar_new());
    adw_header_bar_set_show_back_button(header_bar, TRUE);
    gtk_box_append(navpage_box, GTK_WIDGET(header_bar));

    iface->iface_page = adw_navigation_page_new(GTK_WIDGET(navpage_box), iface->title);

    break;
  }

  adw_navigation_view_add(self->interfaces_view, iface->iface_page);

  iface->iface_row = ADW_PREFERENCES_ROW(adw_action_row_new());
  adw_preferences_row_set_title(iface->iface_row, iface->title);
  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(iface->iface_row), TRUE);
  adw_action_row_add_suffix(ADW_ACTION_ROW(iface->iface_row), GTK_WIDGET(gtk_image_new_from_icon_name("go-next")));
  g_signal_connect(iface->iface_row, "activated", G_CALLBACK(on_iface_activated), iface);

  return GTK_WIDGET(iface->iface_row);
}

static void
network_settings_window_init(NetworkSettingsWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(cssProvider, "/com/plenjos/Settings/theme.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(cssProvider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  self->nm_client = nm_client_new(NULL, NULL);

  if (self->nm_client)
    g_print("NetworkManager version: %s\n", nm_client_get_version(self->nm_client));

  const GPtrArray *devices = nm_client_get_devices(self->nm_client);

  for (size_t i = 0; i < devices->len; i++)
  {
    printf("Device: %s\n", nm_device_get_iface(NM_DEVICE(devices->pdata[i])));

    adw_preferences_group_add(self->interfaces_group, create_net_interface(NM_DEVICE(devices->pdata[i]), self));
  }
  fflush(stdout);
}
