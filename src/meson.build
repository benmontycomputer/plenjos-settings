settings_sources = [
  'main.c',
  'settings-window.c',
  'network-settings/network-settings-window.c',
  'display-settings-window.c',
]

cc = meson.get_compiler('c')

settings_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libhandy-1', version: '>= 1.0.0'),
  dependency('libnm', version: '>= 1.44.0'),
  dependency('libnma', version: '>= 1.10.0'),
  cc.find_library('m', required : true),
]

gnome = import('gnome')

settings_sources += gnome.compile_resources('settings-resources',
  'settings.gresource.xml',
  c_name: 'settings'
)

executable('plenjos-settings', settings_sources,
  dependencies: settings_deps,
  install: true,
)
