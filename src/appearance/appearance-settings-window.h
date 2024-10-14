/* appearance-settings-window.h
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

#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define APPEARANCE_SETTINGS_TYPE_WINDOW (appearance_settings_window_get_type())

G_DECLARE_FINAL_TYPE(AppearanceSettingsWindow, appearance_settings_window, APPEARANCE_SETTINGS, WINDOW, AdwNavigationPage)

G_END_DECLS
