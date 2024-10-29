settings_sources = [
  'main.c',
  'settings-window.c',
  'network/network-settings-window.c',
  'display/display-settings-window.c',
  'appearance/appearance-settings-window.c',
  'panel/panel-settings-window.c',
]

cc = meson.get_compiler('c')

settings_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk4', version: '>= 4.16'),
  dependency('libadwaita-1', version: '>= 1.6.0'),
  dependency('libnm', version: '>= 1.42.0'),
  dependency('libnma-gtk4', version: '>= 1.10.0'),
  dependency('gnome-bluetooth-ui-3.0', version: '>=47.0'),
  cc.find_library('m', required: true),
]

gnome = import('gnome')

settings_sources += gnome.compile_resources('settings-resources', 'settings.gresource.xml', c_name: 'settings')

executable(
  'plenjos-settings',
  settings_sources,
  dependencies: settings_deps,
  install: true,
)