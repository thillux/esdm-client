project('esdm-client', 'cpp')
src = 'main.cpp'

esdm_rpc_client = dependency('esdm_rpc_client')

executable('esdm-client',
  src,
  dependencies: [esdm_rpc_client],
  install: true
)
