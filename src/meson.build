pik_pik_qmr_sources = [
  'main.c',
  'pik-pik_qmr-application.c',
  'pik-pik_qmr-window.c',
]

pik_pik_qmr_deps = [
  dependency('gtk4'),
]

pik_pik_qmr_sources += gnome.compile_resources('pik-pik_qmr-resources',
  'pik-pik_qmr.gresource.xml',
  c_name: 'pik_pik_qmr'
)

executable('pik-pik_qmr', pik_pik_qmr_sources,
  dependencies: pik_pik_qmr_deps,
       install: true,
)
