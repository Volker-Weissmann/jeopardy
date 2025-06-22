project('Jeopardy', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++17']
)

qt_version = get_option('qt_version')


qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'Xml', 'Multimedia', 'MultimediaWidgets'], required: false)
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Xml', 'Multimedia', 'MultimediaWidgets'], required: false)

if qt_version == 'auto'
  if qt6_dep.found()
    qt_version = '6'
  elif qt5_dep.found()
    qt_version = '5'
  else
    error('No supported Qt Version found')  
  endif
endif

if qt_version == '6'
  qt_module = import('qt6')
  qt_dep = qt6_dep
  answer_ui = 'answer_qt6.ui'
elif qt_version == '5'
  qt_module = import('qt5')
  qt_dep = qt5_dep
  answer_ui = 'answer_qt5.ui'
else
  error('Unsupported Qt version: @0@'.format(qt_version))
endif

sources = files(
  'main.cpp',
  'jeopardy.cpp',
  'player.cpp',
  'gamefield.cpp',
  'answer.cpp',
  'editor.cpp',
  'podium.cpp',
  'doublejeopardy.cpp'
)

headers = [
  'jeopardy.h',
  'player.h',
  'gamefield.h',
  'answer.h',
  'editor.h',
  'podium.h',
  'doublejeopardy.h'
]

executable('jeopardy',
  sources: 
    sources
    + qt_module.compile_moc(headers: headers)
    + qt_module.compile_ui(sources: answer_ui),
  dependencies: qt_dep
)