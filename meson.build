project(
  'smartdataconverter',
  'cpp',
  version: '0.1',
  meson_version: '>=0.58.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
  ])

CONVERTER_ARGS=['-DBOOST_URL_NO_SOURCE_LOCATION']

openssl_dep = dependency('openssl', version: '>=1.1.1')
boost_dep = dependency('boost',modules: ['coroutine'])
json_dep = dependency('nlohmann_json')
gtest = subproject('gtest')
reactor=subproject('reactor')
#benchmark = subproject('benchmark')

core_includes = include_directories(['.'])
#if get_option('enable-tests')
#  subdir('tests')
#endif
reactor_dep=reactor.get_variable('reactor_dep')
smartdataconverter = executable('smartdataconverter', 
    'main.cpp', 
    include_directories : core_includes,
    dependencies : [reactor_dep,openssl_dep,json_dep,boost_dep], 
    cpp_args : CONVERTER_ARGS
    )