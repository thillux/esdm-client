
#pragma once
#include <boost/format.hpp>
#include <esdm/esdm_rpc_client.h>
#include <fstream>

// todo: possibility to set testFunction parameters through commandline
void testRandBytes(size_t requestedNumberOfBytes = 32, std::string outfileName = "", int repetitions = 1) {
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestedNumberOfBytes);
	for(int i = 0; i < repetitions; i++){
		esdm_invoke(esdm_rpcc_get_random_bytes(randBytes.data(), randBytes.size()));

		assert(ret == randBytes.size());
		if(outfileName != ""){
			//append randBytes to file
			std::ofstream os;
			os.open(outfileName, std::ios::binary|std::ios::app);
			os.write(reinterpret_cast<const char*>(randBytes.data()), randBytes.size());
		}else{
			std::cout << "0x";
			for (size_t i = 0; i < randBytes.size(); i++) {
				std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
			}
			std::cout << "\n";
		}
	}
	esdm_rpcc_fini_unpriv_service();
}

void testRandBytesFull(size_t requestedNumberOfBytes = 32, std::string outfileName = "", int repetitions = 1) {
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestedNumberOfBytes);
	for(int i = 0; i < repetitions; i++){
		esdm_invoke(
			esdm_rpcc_get_random_bytes_full(randBytes.data(), randBytes.size()));

		assert(ret == randBytes.size());
		if(outfileName != ""){
			//append randBytes to file
			std::ofstream os;
			os.open(outfileName, std::ios::binary|std::ios::app);
			os.write(reinterpret_cast<const char*>(randBytes.data()), randBytes.size());
		}else{
			std::cout << "0x";
			for (size_t i = 0; i < randBytes.size(); i++) {
				std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
			}
			std::cout << "\n";
		}
	}
	esdm_rpcc_fini_unpriv_service();
}

void testRandBytesMin(size_t requestedNumberOfBytes = 32, std::string outfileName = "", int repetitions = 1) {
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestedNumberOfBytes);
	for(int i = 0; i < repetitions; i++){
		esdm_invoke(
			esdm_rpcc_get_random_bytes_min(randBytes.data(), randBytes.size()));

		assert(ret == randBytes.size());
		if(outfileName != ""){
			//append randBytes to file
			std::ofstream os;
			os.open(outfileName, std::ios::binary|std::ios::app);
			os.write(reinterpret_cast<const char*>(randBytes.data()), randBytes.size());
		}else{
			std::cout << "0x";
			for (size_t i = 0; i < randBytes.size(); i++) {
				std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
			}
			std::cout << "\n";
		}
	}
	esdm_rpcc_fini_unpriv_service();
}

void testRandBytesPr(size_t requestedNumberOfBytes = 32, std::string outfileName = "", int repetitions = 1) {
	size_t ret = 0;

	esdm_rpcc_init_unpriv_service(NULL);

	std::vector<uint8_t> randBytes;
	randBytes.resize(requestedNumberOfBytes);
	for(int i = 0; i < repetitions; i++){
		esdm_invoke(
			esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));

		assert(ret == randBytes.size());
		if(outfileName != ""){
			//append randBytes to file
			std::ofstream os;
			os.open(outfileName, std::ios::binary|std::ios::app);
			os.write(reinterpret_cast<const char*>(randBytes.data()), randBytes.size());
		}else{
			std::cout << "0x";
			for (size_t i = 0; i < randBytes.size(); ++i) {
				std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
			}
			std::cout << std::endl;
		}
	}
	esdm_rpcc_fini_unpriv_service();
}

// todo arguments for write data
// void testWriteData(std::vector<uint8_t> data = std::vector<uint8_t>({})) {
void testWriteData(std::string inputString = "") {
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	// we cannot pass an empty string by the user or as the default to the
	// esdm_rpcc_write_data function
	if (inputString == "")
		inputString = "0";
	std::vector<uint8_t> dataToWrite(inputString.begin(), inputString.end());
	esdm_invoke(esdm_rpcc_write_data(dataToWrite.data(), dataToWrite.size()));

	assert(ret == 0);
	esdm_rpcc_fini_unpriv_service();
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

// todo: maybe rename
void testEntCnt() {
	esdm_rpcc_init_unpriv_service(NULL);
	ssize_t ret = 0;
	unsigned int entropyCount = 0;
	esdm_invoke(esdm_rpcc_rnd_get_ent_cnt(&entropyCount));
	if (ret < 0) {
		std::cout << "ret =" << ret << "\n";
	}
	assert(ret == 0);
	std::cout << "entropyCount:" << entropyCount << "\n";
	esdm_rpcc_fini_unpriv_service();
}

void testPrivAddEntropy(std::string inputString = "") {
	ssize_t ret = 0;

	std::cout << "test InputString:" << inputString << "\n";
	esdm_rpcc_init_priv_service(NULL);
	// we cannot pass an empty string by the user or as the default to the
	// esdm_rpcc_rnd_add_entropy function
	if (inputString == "")
		inputString = "0";
	std::vector<uint8_t> randBytes(inputString.begin(), inputString.end());
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

void testPrivAddToEntCnt(unsigned int countToAdd = 1) {
	ssize_t ret = 0;
	esdm_rpcc_init_priv_service(NULL);
	esdm_invoke(esdm_rpcc_rnd_add_to_ent_cnt(countToAdd));
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);
	esdm_rpcc_fini_priv_service();
}

void testPrivClearPool() {
	ssize_t ret = 0;
	esdm_rpcc_init_priv_service(NULL);
	esdm_invoke(esdm_rpcc_rnd_clear_pool());
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);
	esdm_rpcc_fini_priv_service();
}

void testPrivReseedCrng() {
	ssize_t ret = 0;
	esdm_rpcc_init_priv_service(NULL);
	esdm_invoke(esdm_rpcc_rnd_reseed_crng());
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);
	esdm_rpcc_fini_priv_service();
}

// esdm_rpcc functions: /proc handlers
void testGetPoolsize() {
	ssize_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	unsigned int returnedPoolSize = 0;
	esdm_invoke(esdm_rpcc_get_poolsize(&returnedPoolSize));
	if (ret < 0) {
		std::cout << "ret =" << ret << "\n";
	}
	assert(ret == 0);
	std::cout << "poolsize: " << returnedPoolSize << "\n";
	esdm_rpcc_fini_unpriv_service();
}

void testGetWriteWakeupThresh() {
	ssize_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	unsigned int returnedWakupThreshold = 0;
	esdm_invoke(esdm_rpcc_get_write_wakeup_thresh(&returnedWakupThreshold));
	if (ret < 0) {
		std::cout << "ret =" << ret << "\n";
	}
	assert(ret == 0);
	std::cout << "wakeup threshold: " << returnedWakupThreshold << "\n";
	esdm_rpcc_fini_unpriv_service();
}

// need priv service here. else we get errno: 14 (EFAULT)
void testPrivSetWriteWakeupThresh(unsigned int newWakeupThreshold = 0) {
	ssize_t ret = 0;
	esdm_rpcc_init_priv_service(NULL);
	esdm_invoke(esdm_rpcc_set_write_wakeup_thresh(newWakeupThreshold));
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);
	std::cout << "wakeup threshold set to: " << newWakeupThreshold << "\n";
	esdm_rpcc_fini_priv_service();
}

void testGetMinReseedSecs() {
	ssize_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	unsigned int returnedMinReseedSeconds = 0;
	esdm_invoke(esdm_rpcc_get_min_reseed_secs(&returnedMinReseedSeconds));
	if (ret < 0) {
		std::cout << "ret =" << ret << "\n";
	}
	assert(ret == 0);
	std::cout << "min reseed seconds: " << returnedMinReseedSeconds << "\n";
	esdm_rpcc_fini_unpriv_service();
}

// need priv service here. else we get errno: 14 (EFAULT)
void testPrivSetMinReseedSecs(unsigned int newMinReseedSecs = 60) {
	ssize_t ret = 0;
	esdm_rpcc_init_priv_service(NULL);
	esdm_invoke(esdm_rpcc_set_min_reseed_secs(newMinReseedSecs));
	if (ret < 0) {
		std::cout << "ret =" << ret << " . Have you tried running as sudo?\n";
	}
	assert(ret == 0);
	std::cout << "min reseed secondss set to: " << newMinReseedSecs << "\n";
	esdm_rpcc_fini_priv_service();
}
