#include <iostream>
#include <esdm/esdm_rpc_client.h>
#include <cstring>
#include <cassert>
#include <array>
#include <cstdint>

int main(void) {
		std::cout << "ESDM Client" << std::endl;
		
		esdm_rpcc_set_max_online_nodes(1);
		/* Return code irrelevant due to fallback in functions below */
		esdm_rpcc_init_unpriv_service(NULL);
		
		int ret = 0;
		char statusStr[8192];
		memset(statusStr, 0, 8192);

		std::array<uint8_t, 8> randBytes;
		esdm_invoke(esdm_rpcc_get_random_bytes(randBytes.data(), randBytes.size()));

		for(int i = 0; i < randBytes.size(); ++i) {
			std::cout << std::hex << (int) randBytes[i] << std::dec << std::endl;
		}
		
		esdm_rpcc_fini_unpriv_service();
		
		return 0;
}
