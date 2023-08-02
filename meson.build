project(
  'esdm-client',
  'cpp',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'werror=false',
    'warning_level=2',
    'optimization=0',
    'b_sanitize=address,undefined',
    'b_coverage=true'
  ],
  version: '0.1'
)

esdm_rpc_client_dep = dependency('esdm_rpc_client')
boost_dep = dependency('boost', modules : ['program_options'])
json_dep = dependency('jsoncpp')
catch2_deb = dependency('catch2')
kcapi_deb = dependency('libkcapi')

src = [
  'main.cpp'
#  'writer.hpp',
#  'functions.hpp'
]

conf_data = configuration_data()
conf_data.set('JENT_KERNEL', get_option('jentKernel').enabled())
configure_file(
  output : 'config.h',
  configuration : conf_data
)



executable('esdm-client',
  src,
  dependencies: [
    esdm_rpc_client_dep,
    boost_dep,
    json_dep,
    catch2_deb,
    kcapi_deb
  ],
  install: true
)

testsrc = [
  'unitTests.cpp'
]

catch = executable('esdm-test',
  testsrc,
  dependencies: [
    esdm_rpc_client_dep,
    boost_dep,
    json_dep,
    catch2_deb,
    kcapi_deb
  ],
  install: true
)

#test('testMesonCoverage', catch, args : ['~[sudo]'])
#test('testMesonCoverage', catch, args : ['[config]'])