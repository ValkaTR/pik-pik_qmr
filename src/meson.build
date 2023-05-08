pik_pik_qmr_sources = [
  'main.c',
  'pik-pik_qmr-application.c',
  'pik-pik_qmr-window.c',
  'pik-pik_qmr-card_tool.c',
]

pik_pik_qmr_deps = [
  dependency('gtk4', version : '>= 4.10.0', method : 'pkg-config'),
  dependency('gudev-1.0')	# for listing COM ports
]

pik_pik_qmr_sources += gnome.compile_resources('pik-pik_qmr-resources',
  'pik-pik_qmr.gresource.xml',
  c_name: 'pik_pik_qmr'
)

executable('pik-pik_qmr', pik_pik_qmr_sources,
  dependencies: pik_pik_qmr_deps,
       install: true,
)
