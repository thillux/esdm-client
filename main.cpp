#include <array>
#include <boost/format.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <esdm/esdm_rpc_client.h>
#include <iostream>
#include <sys/types.h>

void testUnprivRand() {
  ssize_t ret = 0;

  esdm_rpcc_init_unpriv_service(NULL);

  std::array<uint8_t, 32> randBytes;
  esdm_invoke(
      esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));

  std::cout << "0x";
  for (size_t i = 0; i < randBytes.size(); ++i) {
    std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
  }
  std::cout << std::endl;

  esdm_rpcc_fini_unpriv_service();
}

void testPrivRand() {
  ssize_t ret = 0;

  esdm_rpcc_init_priv_service(NULL);

  std::array<uint8_t, 32> randBytes;
  std::cout << "Warning: this is cyclic entropy feeding for testing purposes and should never be done in production!" << std::endl;
  esdm_invoke(esdm_rpcc_rnd_add_entropy(randBytes.data(), randBytes.size(), randBytes.size() * 8));
  assert(ret == 0);

  esdm_rpcc_fini_priv_service();
}

void testStatus() {
   esdm_rpcc_init_unpriv_service(NULL);

  ssize_t ret = 0;

  std::array<char, 8192> buffer;
  esdm_invoke(
      esdm_rpcc_status(buffer.data(), buffer.size()));

  if(ret == 0) {
    std::string statusStr(buffer.data());
    std::cout << statusStr << std::endl;
  }
  
  esdm_rpcc_fini_unpriv_service();
}

void testSeed() {
   esdm_rpcc_init_unpriv_service(NULL);

  ssize_t ret = 0;

  std::array<uint64_t, 1024 / sizeof(uint64_t)> randBytes;
  esdm_invoke(
      esdm_rpcc_get_seed(reinterpret_cast<uint8_t*>(randBytes.data()), randBytes.size() * sizeof(uint64_t), 0));
  std::cout << "Ret: " << ret << std::endl;

  if(ret > 2 * sizeof(uint64_t)) {
    std::cout << "returned buffer size: " << randBytes[0] << std::endl;
    std::cout << "returned collected bits: " << randBytes[1] << std::endl;

    std::cout << "0x";
    for (size_t i = 2; i < randBytes[0]; ++i) {
      std::cout << boost::format("%02x") % static_cast<int>(reinterpret_cast<uint8_t*>(&randBytes[0])[i]);
    }
    std::cout << std::endl;
  }

  esdm_rpcc_fini_unpriv_service();
}

int main(void) {
  std::cout << "ESDM Client" << std::endl;

  esdm_rpcc_set_max_online_nodes(1);
  
  testStatus();
  //testPrivRand();
  testSeed();
  //testUnprivRand();

  return 0;
}
