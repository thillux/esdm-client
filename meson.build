project(
  'esdm-client',
  'cpp',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'werror=false',
    'warning_level=2',
    'optimization=0',
    'b_sanitize=address,undefined'
  ],
  version: '0.1'
)

esdm_rpc_client_dep = dependency('esdm_rpc_client')
boost_dep = dependency('boost', modules : ['program_options'])

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
