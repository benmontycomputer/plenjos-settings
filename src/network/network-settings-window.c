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
  GList *aps_args;
} NetworkSettingsInterface;

typedef struct WifiArgs
{
  NMAccessPoint *ap;
  NetworkSettingsInterface *iface;
  AdwActionRow *row;
  char *hash;
} WifiArgs;

static void on_iface_activated(AdwActionRow *row, NetworkSettingsInterface *iface)
{
  adw_navigation_view_push(iface->self->interfaces_view, iface->iface_page);
  printf("test\n");
  fflush(stdout);
}

static void
clamp_ap_to_bssid(NMAccessPoint *ap, NMSettingWireless *s_wifi)
{
  const char *str_bssid;
  struct ether_addr *eth_addr;
  GByteArray *bssid;

  /* For a certain list of known ESSIDs which are commonly preset by ISPs
   * and manufacturers and often unchanged by users, lock the connection
   * to the BSSID so that we don't try to auto-connect to your grandma's
   * neighbor's Wi-Fi.
   */

  str_bssid = nm_access_point_get_bssid(ap);
  if (str_bssid)
  {
    eth_addr = ether_aton(str_bssid);
    if (eth_addr)
    {
      bssid = g_byte_array_sized_new(ETH_ALEN);
      g_byte_array_append(bssid, eth_addr->ether_addr_octet, ETH_ALEN);
      g_object_set(G_OBJECT(s_wifi),
                   NM_SETTING_WIRELESS_BSSID, bssid,
                   NULL);
      g_byte_array_free(bssid, TRUE);
    }
  }
}

/*
 * NOTE: this list should *not* contain networks that you would like to
 * automatically roam to like "Starbucks" or "AT&T" or "T-Mobile HotSpot".
 */
static const char *manf_default_ssids[] = {
    "linksys",
    "linksys-a",
    "linksys-g",
    "default",
    "belkin54g",
    "NETGEAR",
    "o2DSL",
    "WLAN",
    "ALICE-WLAN",
    NULL};

static gboolean
is_ssid_in_list(GBytes *ssid, const char **list)
{
  while (*list)
  {
    if (g_bytes_get_size(ssid) == strlen(*list))
    {
      if (!memcmp(*list, g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid)))
        return TRUE;
    }
    list++;
  }
  return FALSE;
}

static gboolean
is_manufacturer_default_ssid(GBytes *ssid)
{
  return is_ssid_in_list(ssid, manf_default_ssids);
}

static GPtrArray *
get_all_connections(NMClient *nm_client)
{
  const GPtrArray *all_connections;
  GPtrArray *connections;
  int i;
  NMConnection *connection;
  NMSettingConnection *s_con;

  all_connections = nm_client_get_connections(nm_client);
  connections = g_ptr_array_new_full(all_connections->len, g_object_unref);

  /* Ignore port connections unless they are wifi connections */
  for (i = 0; i < all_connections->len; i++)
  {
    connection = all_connections->pdata[i];

    s_con = nm_connection_get_setting_connection(connection);
    if (s_con && (!nm_setting_connection_get_master(s_con) || nm_connection_get_setting_wireless(connection)))
      g_ptr_array_add(connections, g_object_ref(connection));
  }

  return connections;
}

static void
activate_existing_cb(GObject *client,
                     GAsyncResult *result,
                     gpointer user_data)
{
  GError *error = NULL;
  NMActiveConnection *active;

  active = nm_client_activate_connection_finish(NM_CLIENT(client), result, &error);
  g_clear_object(&active);
  if (error)
  {
    const char *text = "Failed to activate connection";
    const char *err_text = error->message ? error->message : "Unknown error";

    // utils_show_error_dialog(_("Connection failure"), text, err_text, FALSE, NULL);
    fprintf(stderr, "Connection failure. %s. %s.\n", text, err_text);
    fflush(stderr);
    g_error_free(error);
  }
  // applet_schedule_update_icon(NM_APPLET(user_data));
}

static void
activate_new_cb(GObject *client,
                GAsyncResult *result,
                WifiArgs *args)
{
  GError *error = NULL;
  NMActiveConnection *active;

  active = nm_client_add_and_activate_connection_finish(NM_CLIENT(client), result, &error);
  g_clear_object(&active);
  if (error)
  {
    const char *text = "Failed to add new connection";
    const char *err_text = error->message ? error->message : "Unknown error";

    // utils_show_error_dialog(_("Connection failure"), text, err_text, FALSE, NULL);
    fprintf(stderr, "Connection failure. %s. %s.\n", text, err_text);
    fflush(stderr);
    g_error_free(error);
  }
  // applet_schedule_update_icon(NM_APPLET(user_data));
}

static void
wifi_dialog_response_cb(GtkDialog *foo,
                        gint response,
                        WifiArgs *args)
{
  NMAWifiDialog *dialog = NMA_WIFI_DIALOG(foo);
  // NMApplet *applet = NM_APPLET (user_data);
  NMConnection *connection = NULL, *fuzzy_match = NULL;
  NMDevice *device = NULL;
  NMAccessPoint *ap = NULL;
  GPtrArray *all;
  int i;

  if (response != GTK_RESPONSE_OK)
    goto done;

  /* nma_wifi_dialog_get_connection() returns a connection with the
   * refcount incremented, so the caller must remember to unref it.
   */
  connection = nma_wifi_dialog_get_connection(dialog, &device, &ap);
  g_assert(connection);
  g_assert(device);

  /* Find a similar connection and use that instead */
  all = get_all_connections(args->iface->self->nm_client);
  for (i = 0; i < (int)all->len; i++)
  {
    if (nm_connection_compare(connection,
                              NM_CONNECTION(all->pdata[i]),
                              (NM_SETTING_COMPARE_FLAG_FUZZY | NM_SETTING_COMPARE_FLAG_IGNORE_ID)))
    {
      fuzzy_match = NM_CONNECTION(all->pdata[i]);
      printf("match");
      fflush(stdout);
      break;
    }
  }
  g_ptr_array_unref(all);

  if (fuzzy_match)
  {
    nm_client_activate_connection_async(args->iface->self->nm_client,
                                        fuzzy_match,
                                        device,
                                        ap ? nm_object_get_path(NM_OBJECT(ap)) : NULL,
                                        NULL,
                                        activate_existing_cb,
                                        args);
  }
  else
  {
    NMSetting *s_con;
    NMSettingWireless *s_wifi = NULL;
    const char *mode = NULL;

    /* Entirely new connection */

    /* Don't autoconnect adhoc networks by default for now */
    s_wifi = nm_connection_get_setting_wireless(connection);
    if (s_wifi)
      mode = nm_setting_wireless_get_mode(s_wifi);
    if (g_strcmp0(mode, "adhoc") == 0 || g_strcmp0(mode, "ap") == 0)
    {
      s_con = nm_connection_get_setting(connection, NM_TYPE_SETTING_CONNECTION);
      if (!s_con)
      {
        s_con = nm_setting_connection_new();
        nm_connection_add_setting(connection, s_con);
      }
      g_object_set(G_OBJECT(s_con), NM_SETTING_CONNECTION_AUTOCONNECT, FALSE, NULL);
    }

    nm_client_add_and_activate_connection_async(args->iface->self->nm_client,
                                                connection,
                                                device,
                                                ap ? nm_object_get_path(NM_OBJECT(ap)) : NULL,
                                                NULL,
                                                (GAsyncReadyCallback)activate_new_cb,
                                                args);
  }

  /* Balance nma_wifi_dialog_get_connection() */
  g_object_unref(connection);

done:
  gtk_window_close(GTK_WINDOW(dialog));
}

static void
wifi_dialog_response(GtkDialog *foo,
                     gint response,
                     WifiArgs *args)
{
  NMAWifiDialog *dialog = NMA_WIFI_DIALOG(foo);
  NMConnection *connection = NULL, *fuzzy_match = NULL;
  NMDevice *device = NULL;
  NMAccessPoint *ap = NULL;
  GPtrArray *all;

  if (response != GTK_RESPONSE_OK)
    goto done;

  /* nma_wifi_dialog_get_connection() returns a connection with the
   * refcount incremented, so the caller must remember to unref it.
   */
  connection = nma_wifi_dialog_get_connection(dialog, &device, &ap);
  g_assert(connection);
  g_assert(device);

  NMSetting *s_con = NULL;
  NMSettingWireless *s_wifi = NULL;
  const char *mode = NULL;

  /* Don't autoconnect adhoc networks by default for now */
  s_wifi = nm_connection_get_setting_wireless(connection);
  if (s_wifi)
    mode = nm_setting_wireless_get_mode(s_wifi);
  if (g_strcmp0(mode, "adhoc") == 0 || g_strcmp0(mode, "ap") == 0)
  {
    s_con = nm_connection_get_setting(connection, NM_TYPE_SETTING_CONNECTION);
    if (!s_con)
    {
      s_con = nm_setting_connection_new();
      nm_connection_add_setting(connection, s_con);
    }
    g_object_set(G_OBJECT(s_con), NM_SETTING_CONNECTION_AUTOCONNECT, FALSE, NULL);
  }

  nm_client_add_and_activate_connection_async(args->iface->self->nm_client,
                                              connection,
                                              device,
                                              ap ? nm_object_get_path(NM_OBJECT(ap)) : NULL,
                                              NULL,
                                              (GAsyncReadyCallback)activate_existing_cb,
                                              args);

  /* Balance nma_wifi_dialog_get_connection() */
  g_object_unref(connection);

done:
  gtk_window_close(GTK_WINDOW(dialog));
}

static void on_wifi_activated(AdwActionRow *row, WifiArgs *args)
{
  NMConnection *connection = nm_simple_connection_new();

  NMAccessPoint *ap = args->ap;

  NMSettingConnection *s_con = NULL;
  NMSettingWireless *s_wifi = NULL;
  NMSettingWirelessSecurity *s_wsec = NULL;
  NMSetting8021x *s_8021x = NULL;
  GBytes *ssid = NULL;
  NM80211ApSecurityFlags wpa_flags, rsn_flags;
  GtkWidget *dialog = NULL;
  char *uuid;

  s_con = (NMSettingConnection *)nm_setting_connection_new();
  nm_connection_add_setting(connection, NM_SETTING(s_con));

  // GtkWidget *dialog = nma_wifi_dialog_new(args->iface->self->nm_client, connection, args->iface->device, args->ap, FALSE);

  s_wifi = (NMSettingWireless *)nm_setting_wireless_new();

  ssid = nm_access_point_get_ssid(ap);
  if ((nm_access_point_get_mode(ap) == NM_802_11_MODE_INFRA) && (is_manufacturer_default_ssid(ssid) == TRUE))
  {

    /* Lock connection to this AP if it's a manufacturer-default SSID
     * so that we don't randomly connect to some other 'linksys'
     */
    clamp_ap_to_bssid(ap, s_wifi);
  }

  /* Need a UUID for the "always ask" stuff in the Dialog of Doom */
  uuid = nm_utils_uuid_generate();
  g_object_set(s_con, NM_SETTING_CONNECTION_UUID, uuid, NULL);
  g_free(uuid);

  g_object_set(s_wifi,
               NM_SETTING_WIRELESS_SSID, ssid,
               NULL);

  nm_connection_add_setting(connection, NM_SETTING(s_wifi));

  rsn_flags = nm_access_point_get_rsn_flags(ap);
  wpa_flags = nm_access_point_get_wpa_flags(ap);
  if ((rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X) || (wpa_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X))
  {
    s_wsec = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    g_object_set(s_wsec, NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, "wpa-eap", NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_wsec));

    s_8021x = (NMSetting8021x *)nm_setting_802_1x_new();
    nm_setting_802_1x_add_eap_method(s_8021x, "ttls");
    g_object_set(s_8021x, NM_SETTING_802_1X_PHASE2_AUTH, "mschapv2", NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_8021x));
  }

  dialog = nma_wifi_dialog_new(args->iface->self->nm_client, connection, args->iface->device, ap, FALSE);

  if (dialog)
  {
    g_signal_connect(dialog, "response",
                     G_CALLBACK(wifi_dialog_response_cb),
                     args);
  }

  gtk_window_present(GTK_WINDOW(dialog));
}

/* List known trojan networks that should never be shown to the user */
static const char *denylisted_ssids[] = {
    /* http://www.npr.org/templates/story/story.php?storyId=130451369 */
    "Free Public Wi-Fi",
    NULL};

static gboolean
is_denylisted_ssid(GBytes *ssid)
{
  return is_ssid_in_list(ssid, denylisted_ssids);
}

struct dup_data
{
  NetworkSettingsInterface *iface;
  WifiArgs *found;
  char *hash;
};

static void
find_duplicate(gpointer d, gpointer user_data)
{
  struct dup_data *data = (struct dup_data *)user_data;
  NMDevice *device;
  const char *hash;
  WifiArgs *args = (WifiArgs *)d;

  g_assert(d && args);
  g_return_if_fail(data);
  g_return_if_fail(data->hash);

  if (data->found /* || !ADW_IS_ACTION_ROW (widget) */)
    return;

  device = args->iface->device;
  if (NM_DEVICE(device) != data->iface->device)
    return;

  hash = args->hash;
  if (hash && (strcmp(hash, data->hash) == 0))
    data->found = args;
}

static void add_ap(gpointer ap_ptr, NetworkSettingsInterface *iface)
{
  NMAccessPoint *ap = NM_ACCESS_POINT(ap_ptr);

  GBytes *ssid;
  struct dup_data dup_data = {NULL, NULL, NULL};

  /* Don't add BSSs that hide their SSID or are denylisted */
  ssid = nm_access_point_get_ssid(ap);
  if (!ssid || nm_utils_is_empty_ssid(g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid)) || is_denylisted_ssid(ssid))
    return;

  /* Find out if this AP is a member of a larger network that all uses the
   * same SSID and security settings.  If so, we'll already have a menu item
   * for this SSID, so just update that item's strength and add this AP to
   * menu item's duplicate list.
   */
  dup_data.found = NULL;
  dup_data.hash = g_object_get_data(G_OBJECT(ap), "hash");
  g_return_if_fail(dup_data.hash != NULL);

  dup_data.iface = iface;
  g_list_foreach(iface->aps_args, find_duplicate, &dup_data);

  if (dup_data.found)
  {
    // nm_network_menu_item_set_strength (dup_data.found, nm_access_point_get_strength (ap), applet);
    // nm_network_menu_item_add_dupe (dup_data.found, ap);
    return;
  }

  AdwActionRow *row = ADW_ACTION_ROW(adw_action_row_new());
  GBytes *ssid_bytes = nm_access_point_get_ssid(ap);
  char *ssid_str = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid_bytes, NULL), g_bytes_get_size(ssid_bytes));
  adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), ssid_str);
  free(ssid_str);
  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), TRUE);
  adw_action_row_add_suffix(ADW_ACTION_ROW(row), GTK_WIDGET(gtk_image_new_from_icon_name("go-next")));

  WifiArgs *args = malloc(sizeof(WifiArgs));
  args->ap = ap;
  args->iface = iface;
  args->row = row;
  args->hash = strdup(dup_data.hash);
  g_signal_connect(row, "activated", G_CALLBACK(on_wifi_activated), args);

  iface->aps_args = g_list_append(iface->aps_args, args);

  adw_preferences_group_add(iface->wifi_group, GTK_WIDGET(row));
}

static void remove_ap_gtk(AdwActionRow *row)
{
  gtk_list_box_remove(GTK_LIST_BOX(gtk_widget_get_parent(GTK_WIDGET(row))), GTK_WIDGET(row));
}

static void on_ap_add(NMDeviceWifi *device, NMAccessPoint *ap, NetworkSettingsInterface *iface)
{
  printf("test\n\n");
  fflush(stdout);
  add_ap(ap, iface);
}

static void on_ap_remove(NMDeviceWifi *device, NMAccessPoint *ap, NetworkSettingsInterface *iface)
{
  printf("test22\n\n");
  fflush(stdout);
  GList *item = iface->aps_args;
  while (item)
  {
    WifiArgs *args = (WifiArgs *)item->data;

    if (args->ap == ap)
    {
      iface->aps_args = g_list_remove(iface->aps_args, args);

      g_idle_add(G_SOURCE_FUNC(remove_ap_gtk), args->row);

      free(args);
      break;
    }

    item = item->next;
  }
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
  iface->aps_args = NULL;

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

    nm_device_wifi_request_scan(NM_DEVICE_WIFI(device), NULL, NULL);

    iface->aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));

    g_ptr_array_foreach(iface->aps, (GFunc)add_ap, iface);

    g_signal_connect(iface->device, "access_point_added", G_CALLBACK(on_ap_add), iface);
    g_signal_connect(iface->device, "access_point_removed", G_CALLBACK(on_ap_remove), iface);
    // NMConnection *connection = NM_CONNECTION (nm_client_get_primary_connection (self->nm_client));
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
