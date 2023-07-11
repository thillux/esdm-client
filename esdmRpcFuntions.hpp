
#pragma once
#include <boost/format.hpp>
#include <esdm/esdm_rpc_client.h>

// todo implement functions from esdm_rpc_client.h
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

//todo: finish function calls

// void testUnprivRandBytesFull() {
// 	size_t ret = 0;
// 	esdm_rpcc_init_unpriv_service(NULL);
// 	esdm_rpcc_fini_unpriv_service();
// }
// void testUnprivRandBytesMin() {
// 	size_t ret = 0;
// 	esdm_rpcc_init_unpriv_service(NULL);
// 	esdm_rpcc_fini_unpriv_service();
// }
// void testUnprivRandBytes() {
// 	size_t ret = 0;
// 	esdm_rpcc_init_unpriv_service(NULL);
// 	esdm_rpcc_fini_unpriv_service();
// }
// void testUnprivWriteData() {
// 	size_t ret = 0;
// 	esdm_rpcc_init_unpriv_service(NULL);
// 	esdm_rpcc_fini_unpriv_service();
// }

//todo: maybe rename
void testEntCnt() {
	esdm_rpcc_init_unpriv_service(NULL);
	size_t ret = 0;
	unsigned int entropyCount = 0;
	esdm_invoke(esdm_rpcc_rnd_get_ent_cnt(&entropyCount));
	std::cout << "ret1:" << ret << "\n";
    assert(ret == 0);
	// esdm_invoke(esdm_rpcc_rnd_reseed_crng());
	// std::cout << "ret2:" << ret << "\n";
	// esdm_invoke(esdm_rpcc_rnd_clear_pool());
	// std::cout << "ret3:" << ret << "\n";
	std::cout << "entropyCount:" << entropyCount << "\n";
	esdm_rpcc_fini_unpriv_service();
}

// void testPrivAddToEntCnt() {
// 	ssize_t ret = 0;
// 	esdm_rpcc_init_priv_service(NULL);
// 	esdm_invoke(esdm_rpcc_rnd_add_to_ent_cnt());
// 	assert(ret == 0);
// 	esdm_rpcc_fini_priv_service();
// }
// void testPrivClearPool() {
// 	ssize_t ret = 0;
// 	esdm_rpcc_init_priv_service(NULL);
// 	esdm_invoke(esdm_rpcc_rnd_clear_pool());
// 	assert(ret == 0);
// 	esdm_rpcc_fini_priv_service();
// }
// void testPrivReseedCrng() {
// 	ssize_t ret = 0;
// 	esdm_rpcc_init_priv_service(NULL);
// 	esdm_invoke(esdm_rpcc_rnd_reseed_crng());
// 	assert(ret == 0);
// 	esdm_rpcc_fini_priv_service();
// }

void testPrivRand() {
	ssize_t ret = 0;

	esdm_rpcc_init_priv_service(NULL);

	std::array<uint8_t, 32> randBytes;
	std::cout << "Warning: this is cyclic entropy feeding for testing purposes "
				 "and should never be done in production!"
			  << std::endl;
	esdm_invoke(esdm_rpcc_rnd_add_entropy(randBytes.data(), randBytes.size(),
										  randBytes.size() * 8));
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);

	esdm_rpcc_fini_priv_service();
}

void testStatus() {
	esdm_rpcc_init_unpriv_service(NULL);

	ssize_t ret = 0;

	std::array<char, 8192> buffer;
	esdm_invoke(esdm_rpcc_status(buffer.data(), buffer.size()));
	assert(ret == 0);
	if (ret == 0) {
		std::string statusStr(buffer.data());
		std::cout << statusStr << std::endl;
	}

	esdm_rpcc_fini_unpriv_service();
}

void testSeed() {
	esdm_rpcc_init_unpriv_service(NULL);

	size_t ret = 0;

	std::array<uint64_t, 1024 / sizeof(uint64_t)> randBytes;
	esdm_invoke(esdm_rpcc_get_seed(reinterpret_cast<uint8_t*>(randBytes.data()),
								   randBytes.size() * sizeof(uint64_t), 0));
	std::cout << "Ret: " << ret << std::endl;

	assert(ret > 2 * sizeof(uint64_t));
	if (ret > 2 * sizeof(uint64_t)) {
		std::cout << "returned buffer size: " << randBytes[0] << std::endl;
		std::cout << "returned collected bits: " << randBytes[1] << std::endl;

		std::cout << "0x";
		for (size_t i = 2; i < randBytes[0]; ++i) {
			std::cout << boost::format("%02x") %
							 static_cast<int>(
								 reinterpret_cast<uint8_t*>(&randBytes[0])[i]);
		}
		std::cout << std::endl;
	}

	esdm_rpcc_fini_unpriv_service();
}
