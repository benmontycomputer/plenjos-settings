/* settings-window.c
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
#include "settings-window.h"

struct _SettingsWindow
{
  AdwApplicationWindow parent_instance;

  GtkStackSwitcher *stack_switcher;
  GtkStack *main_stack;
  AdwNavigationSplitView *split_view;
};

G_DEFINE_TYPE(SettingsWindow, settings_window, ADW_TYPE_APPLICATION_WINDOW)

static void
settings_window_class_init(SettingsWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Settings/settings-window.ui");
  gtk_widget_class_bind_template_child(widget_class, SettingsWindow, stack_switcher);
  gtk_widget_class_bind_template_child(widget_class, SettingsWindow, main_stack);
  gtk_widget_class_bind_template_child(widget_class, SettingsWindow, split_view);
}

void stack_switcher_set_halign_helper(GtkWidget *widget,
                                      gpointer data)
{
  gtk_widget_set_halign(widget, GTK_ALIGN_START);
}

void stack_switcher_set_halign(GtkWidget *widget,
                               gpointer data)
{
  // gtk_container_foreach (widget, stack_switcher_set_halign_helper, NULL);
}

typedef struct
{
  char *name;
  SettingsWindow *self;
  GtkButton *item;
} StackItemSwitchHelperArgs;

gboolean stack_item_switch_helper(GtkWidget *item,
                                  StackItemSwitchHelperArgs *args)
{
  gtk_stack_set_visible_child_name(args->self->main_stack, args->name);

  adw_navigation_split_view_set_show_content(args->self->split_view, TRUE);

  // adw_header_bar_set_title (args->self->secondary_header_bar, args->name);

  return TRUE;
}

void stack_item_highlight_helper(GtkWidget *thingy,
                                 GtkWidget *widget,
                                 StackItemSwitchHelperArgs *args)
{
  const char *visible_child_name = gtk_stack_get_visible_child_name(args->self->main_stack);

  if (!args->name || !visible_child_name)
  {
    return;
  }

  if (!strcmp(visible_child_name, args->name))
  {
    gtk_widget_set_state_flags(GTK_WIDGET(args->item), GTK_STATE_FLAG_SELECTED, TRUE);
  }
  else
  {
    gtk_widget_set_state_flags(GTK_WIDGET(args->item), GTK_STATE_FLAG_NORMAL, TRUE);
  }
}

GtkWidget *create_stack_item(SettingsWindow *self,
                             char *name,
                             char *display_name,
                             char *icon_name)
{
  GtkButton *item = GTK_BUTTON(gtk_button_new());
  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4));
  gtk_widget_set_name(GTK_WIDGET(item), "settings_stack_item");
  gtk_box_append(box, gtk_image_new_from_icon_name(icon_name));
  gtk_box_append(box, gtk_label_new(display_name));
  gtk_button_set_child(item, GTK_WIDGET(box));

  StackItemSwitchHelperArgs *args = malloc(sizeof(StackItemSwitchHelperArgs));
  args->name = name;
  args->self = self;
  args->item = item;

  g_signal_connect(item, "clicked", (GCallback)stack_item_switch_helper, args);
  g_signal_connect(self->main_stack, "notify::visible-child", (GCallback)stack_item_highlight_helper, args);

  return GTK_WIDGET(item);
}

static void
settings_window_init(SettingsWindow *self)
{
  adw_init();

  gtk_widget_init_template(GTK_WIDGET(self));

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(cssProvider, "/com/plenjos/Settings/theme.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(cssProvider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  // TODO: flip the close buttons to the other header bar depending on where they should be

  GtkBox *box = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(self->stack_switcher)));

  NetworkSettingsWindow *network_settings = g_object_new(NETWORK_SETTINGS_TYPE_WINDOW, NULL);
  DisplaySettingsWindow *display_settings = g_object_new(DISPLAY_SETTINGS_TYPE_WINDOW, NULL);

  // adw_header_bar_set_title(self->secondary_header_bar, "Network Settings");

  gtk_stack_add_titled(self->main_stack, GTK_WIDGET(network_settings), "Network Settings", "Network Settings");
  gtk_box_append(box, create_stack_item(self, "Network Settings", "Network Settings", "preferences-system-network"));

  gtk_stack_add_titled(self->main_stack, GTK_WIDGET(display_settings), "Display Settings", "Display Settings");
  gtk_box_append(box, create_stack_item(self, "Display Settings", "Display Settings", "preferences-desktop-display"));

  gtk_stack_add_titled(self->main_stack, gtk_label_new("Test 3"), "test3", "Test3");
  gtk_box_append(box, create_stack_item(self, "test3", "Test3", "preferences-system"));

  GValue network_icon_name = G_VALUE_INIT;
  g_value_init(&network_icon_name, G_TYPE_STRING);
  g_value_set_string(&network_icon_name, "preferences-system-network");

  // gtk_container_child_set_property (GTK_CONTAINER (self->main_stack), GTK_WIDGET (network_settings), "icon-name", &network_icon_name);
  // gtk_container_foreach(GTK_CONTAINER(self->main_stack), stack_switcher_set_halign, NULL);
}
