#include <array>
#include <boost/format.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <esdm/esdm_rpc_client.h>
#include <iostream>

int main(void) {
  std::cout << "ESDM Client" << std::endl;

  esdm_rpcc_set_max_online_nodes(1);
  /* Return code irrelevant due to fallback in functions below */
  esdm_rpcc_init_unpriv_service(NULL);

  int ret = 0;
  std::array<char, 8192> statusStr;
  memset(statusStr.data(), 0, statusStr.size());
  esdm_invoke(esdm_rpcc_status(statusStr.data(), statusStr.size()));
  std::cout << statusStr.data() << std::endl;

  std::array<uint8_t, 8> randBytes;
  esdm_invoke(
      esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));

  std::cout << "0x";
  for (size_t i = 0; i < randBytes.size(); ++i) {
    std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
  }
  std::cout << std::endl;

  esdm_rpcc_fini_unpriv_service();

  return 0;
}
