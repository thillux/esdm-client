project(
  'esdm-client',
  'cpp',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'werror=true',
    'warning_level=2'
  ],
  version: '0.1'
)

esdm_rpc_client = dependency('esdm_rpc_client')
boost_dep = dependency('boost', modules : ['thread', 'utility'])

src = [
  'main.cpp'
]

executable('esdm-client',
  src,
  dependencies: [
    esdm_rpc_client_dep,
    boost_dep
  ],
  install: true
)
