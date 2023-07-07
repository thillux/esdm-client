#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <esdm/esdm_rpc_client.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <optional>

enum class benchmarkType { timeGetRandom, meassureEntropy, testTimeToSeed, noType };

using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using meassureEntropyOutput = std::vector<std::pair<int64_t, int>>;

// #define for_each_rep(rep) for(size_t i = 0; i < rep;  i++)

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
	std::cout << "Warning: this is cyclic entropy feeding for testing purposes "
				"and should never be done in production!"
			<< std::endl;
	esdm_invoke(esdm_rpcc_rnd_add_entropy(randBytes.data(), randBytes.size(),
										randBytes.size() * 8));
	assert(ret == 0);

	esdm_rpcc_fini_priv_service();
}

void testStatus() {
	esdm_rpcc_init_unpriv_service(NULL);

	ssize_t ret = 0;

	std::array<char, 8192> buffer;
	esdm_invoke(esdm_rpcc_status(buffer.data(), buffer.size()));

	if (ret == 0) {
		std::string statusStr(buffer.data());
		std::cout << statusStr << std::endl;
	}

	esdm_rpcc_fini_unpriv_service();
}

void testSeed() {
	std::cout << "test\n";
	esdm_rpcc_init_unpriv_service(NULL);

	size_t ret = 0;

	std::array<uint64_t, 1024 / sizeof(uint64_t)> randBytes;
	esdm_invoke(esdm_rpcc_get_seed(reinterpret_cast<uint8_t *>(randBytes.data()),
									randBytes.size() * sizeof(uint64_t), 0));
	std::cout << "Ret: " << ret << std::endl;

	if (ret > 2 * sizeof(uint64_t)) {
		std::cout << "returned buffer size: " << randBytes[0] << std::endl;
		std::cout << "returned collected bits: " << randBytes[1] << std::endl;

		std::cout << "0x";
		for (size_t i = 2; i < randBytes[0]; ++i) {
		std::cout << boost::format("%02x") %
						static_cast<int>(
							reinterpret_cast<uint8_t *>(&randBytes[0])[i]);
		}
		std::cout << std::endl;
	}

	esdm_rpcc_fini_unpriv_service();
}

void getRandomNumbers(std::vector<std::pair<size_t, std::string>> &returnVec,
					  const int requestSize = 32) {
	size_t ret = 0;
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestSize);
	esdm_invoke(
		esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
	assert(ret > 0);
	std::string returnString;
	returnString.append("0x");
	for (size_t i = 0; i < randBytes.size(); ++i) {
		returnString.append(str(boost::format("%02x") % static_cast<int>(randBytes[i])));
	}
	returnVec.push_back(std::make_pair(ret, returnString));
}

void getRandomNumbers(const int requestSize = 32, const bool coutOn = false) {
	size_t ret = 0;
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestSize);
	esdm_invoke(
			esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
	assert(ret > 0);

	// todo: remove print before actual using this in benchmarking!
	if (coutOn) {
	std::cout << "retValue:" << ret << "\n";
		if (ret > 0) {
			std::cout << "0x";
			for (size_t i = 0; i < randBytes.size(); ++i) {
				std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
			}
			std::cout << "\n";
		}
	}
}

// todo: save the output of the getRandomNumbers call and log to file? -> use
// if(v != nullptr)
int64_t benchmarkGetRandom(std::vector<std::pair<size_t, std::string>> &returnVec,
						bool saveRandomOutput = false, const size_t requests = 1, const int requestSize = 32) {
	esdm_rpcc_init_unpriv_service(NULL);
	std::chrono::time_point<std::chrono::system_clock> start = Clock::now();
	int round = 0;
	for (size_t i = 0; i < requests; i++)
	{
		if (saveRandomOutput) {
			getRandomNumbers(returnVec, requestSize);
		}else
			getRandomNumbers(requestSize);
		round++;
	}
	std::chrono::time_point<std::chrono::system_clock> end = Clock::now();

	auto duration = end - start;
	ns durationNs = std::chrono::duration_cast<ns>(duration);

	esdm_rpcc_fini_unpriv_service();
	return durationNs.count();
}

// todo
void benchmarkMeassureEntropy(meassureEntropyOutput &returnVec, const long observationTime = -1, const long deltaMs = -1) {
	// todo:
	//-do a call OR wait? -> maybe call external programm get_raw_entropy with
	// system()
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	char buffer[2048];
	assert(ret == 0);
	bool fullySeeded = false;
	bool firstLoop = true;
	bool useDelta = deltaMs != -1 ? true : false;
	ns maxDuration(0);
	if (observationTime > 0)
		maxDuration = std::chrono::duration_cast<ns>(ms(observationTime));
	ns duration = ns(0);
	std::chrono::time_point<std::chrono::system_clock, ns> start = Clock::now();
	std::chrono::time_point<std::chrono::system_clock, ns> now = Clock::now();
	ns delta;
	if (useDelta)
		delta = std::chrono::duration_cast<ns>(us(deltaMs));
	std::chrono::time_point<std::chrono::system_clock, ns> deltaTimePoint = Clock::now();

	// save start date as the first tuple in our retVec: (startTime, -1)
	int64_t startNano = start.time_since_epoch().count();
	returnVec.push_back(std::make_pair(startNano, -1));

	// if observationTime is set to the default -1 then duration == maxDuration
	//-> the loop will repeat till the esdm status says it is fully seeded
	while (!fullySeeded || (duration < maxDuration)) {
		deltaTimePoint = Clock::now();
		if (std::chrono::duration_cast<ns>(deltaTimePoint - now) < delta && !firstLoop &&
			useDelta) {
			continue;
		}
		now = Clock::now();

		/*
			todo: maybe call a function here to manipulate the entropy level, if
			possible. (like reset, decrease the entropy) maybe get_raw_entropy?
		*/

		// unsigned int entCnt;
		// esdm_invoke(esdm_rpcc_rnd_get_ent_cnt(&entCnt));
		// std::cout << "entCnt:" << entCnt << "\n";

		esdm_invoke(esdm_rpcc_status(buffer, sizeof(buffer)));
		duration = std::chrono::duration_cast<ns>(now - start);
		// try to find if esdm is fully seeded
		char searchFullySeeded[] = "ESDM fully seeded: true";
		fullySeeded = strstr(buffer, searchFullySeeded) != NULL ? true : false;

		// get the current entropy level through string parsing
		char searchEntropyLevel[] = "entropy level: ";
		char *bufferPointer = strstr(buffer, searchEntropyLevel);
		char *entropyLevelCstr = std::strtok(bufferPointer, "\n") + strlen(searchEntropyLevel);
		int entropyLevel = atoi(entropyLevelCstr);
		int64_t nowNano = now.time_since_epoch().count();

		// std::cout << "nowNano:" << nowNano << "\n";

		// std::cout << "status ent level:" << entropyLevel << "\n";

		std::pair<int64_t, int> entry = std::make_pair(nowNano, entropyLevel);
		returnVec.push_back(entry);
		std::memset(buffer, 0, sizeof(buffer));
		firstLoop = false;
	}

	esdm_rpcc_fini_unpriv_service();
}

int64_t timeTillSeeded() {
	std::chrono::time_point<std::chrono::system_clock, ns> start = Clock::now();
	std::chrono::time_point<std::chrono::system_clock, ns> end = start;
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	char buffer[2048];
	assert(ret == 0);
	bool fullySeeded = false;
	bool firstLoop = true;


	std::array<uint8_t, 64> randomOutputBuffer;
	esdm_invoke(esdm_rpcc_get_random_bytes_pr(randomOutputBuffer.data(), randomOutputBuffer.size()));

	while (!fullySeeded) {
	esdm_invoke(esdm_rpcc_status(buffer, sizeof(buffer)));
	char searchFullySeeded[] = "ESDM fully seeded: true";
	fullySeeded = strstr(buffer, searchFullySeeded) != NULL ? true : false;
		if (fullySeeded && firstLoop) {
			firstLoop = false;
			break;
		}
	std::memset(buffer, 0, sizeof(buffer));
	if (fullySeeded)
		end = Clock::now();
	}

	ns duration = end - start;

	esdm_rpcc_fini_unpriv_service();
	return duration.count();
}

int start_getRandom(const std::vector<std::string> &optionVec, const std::string outputFileName,
				const bool saveRandomOutput = false) {
	// execute 'timeGetRandom' benchmark
	int64_t outputDuration = 0;
	if (optionVec.size() > 3) {
		std::cout << "to many arguments for the timeGetRandom benchmark\n. Expected at most 3 "
					"Arguments:\n"
				<< "(0) type: 'timeGetRandom'\n"
				<< "(1) requests\n"
				<< "(2) requestSize\n";
		return -1;
	}

	std::vector<std::pair<size_t, std::string>> retVec;
	if (optionVec.size() == 3) {
		outputDuration = benchmarkGetRandom(retVec, saveRandomOutput, std::stoi(optionVec[1]), std::stoi(optionVec[2]));
	} else if (optionVec.size() == 2) {
		outputDuration = benchmarkGetRandom(retVec, saveRandomOutput, std::stoi(optionVec[1]));
	} else {
		outputDuration = benchmarkGetRandom(retVec, saveRandomOutput);
	}
	// save result
	FILE *f = fopen(outputFileName.c_str(), "w");
	if (!f) {
		std::cout << "Could not open output file.\n";
		return -1;
	}
	fprintf(f, "duration:\t%lu\n", outputDuration);

	for (auto i : retVec) {
		fprintf(f, "%lu\t%s\n", i.first, i.second.c_str());
	}
	fclose(f);
	return 0;
}

int start_meassureEntropy(const std::vector<std::string> &optionVec, std::string outputFileName) {
	// execute 'meassureEntropy' benchmark
	meassureEntropyOutput output;
	if (optionVec.size() > 3) {
		std::cout
			<< "to many arguments for the meassureEntropy benchmark\n. Expected at most 3 "
			"Arguments:\n"
			<< "(0) type: 'meassureEntropy'\n"
			<< "(1) duration: check the entropy level for 'duration' milliseconds\n"
			<< "(2) delta: if set only check after 'delta' microseconds have "
			"passed\n";
		return -1;
	} else if (optionVec.size() == 3) {
		benchmarkMeassureEntropy(output, std::stoi(optionVec[1]), std::stoi(optionVec[2]));
	} else if (optionVec.size() == 2) {
		benchmarkMeassureEntropy(output, std::stoi(optionVec[1]));
	} else if (optionVec.size() == 1) {
		benchmarkMeassureEntropy(output);
	}
	// save result
	FILE *f = fopen(outputFileName.c_str(), "w");
	if (!f) {
		std::cout << "Could not open output file.\n";
		return -1;
	}
	for (auto i : output) {
		fprintf(f, "%lu\t%d\n", i.first, i.second);
	}
	fclose(f);
	return 0;
}

int start_timeToSeed(const std::vector<std::string> &optionVec, std::string outputFileName) {
	int64_t outputTimeToSeed;
	// execute 'testTimeToSeed' benchmark
	if (optionVec.size() > 1) {
		std::cout << "to many arguments for the meassureEntropy benchmark\n. Expected at most 1 "
					"Argument:\n"
				<< "(0) type: 'testTimeToSeed'\n";
	} else if (optionVec.size() == 1) {
		outputTimeToSeed = timeTillSeeded();
	}
	// save result
	FILE *f = fopen(outputFileName.c_str(), "w");
	if (!f) {
		std::cout << "Could not open output file.\n";
		return -1;
	}
	int ret = 0;
	ret = fprintf(f, "%lu\n", outputTimeToSeed);
	assert(ret > 0);
	return fclose(f);
}

int call_benchmark(const std::vector<std::string> &optionVec,
					const boost::program_options::variables_map &vm) {
	std::cout << "vector.size():" << optionVec.size() << "\n";

	bool saveOutput = vm["save"].as<bool>();

	if (optionVec.size() == 0) {
		std::cout
			<< "please specify a benchmark to run:\n"
			<< "'timeGetRandom(requests,requestSize)':\tCall esdm_rpcc_get_random_bytes_pr\n"
			<< "\t requests:\tNumber of times the function is called\n"
			<< "\t requestSize:\tAmount of requested random bytes\n"
			<< "'meassureEntropy(observationTime,deltaMs)':\t Read the entropy status of the esdm\n"
			<< "\t observationTime:\tFor how long the entropy status is meassured (default: "
			"until fully seeded)\n"
			<< "\t deltaMs:\tOnly read the entropy status after 'deltaMs' "
			"milliseconds (default: 0) have passed since the last observationTime\n"
			<< "'testTimeToSeed()':\tMeassure the time till esdm is fully seeded\n";
		return -1;
	}

	int repetitions = -1;
	if (vm.count("r")) {
		repetitions = vm["r"].as<int>();
	}

	benchmarkType type;
	std::string typeStr = optionVec[0];
	if (typeStr == "timeGetRandom")
		type = benchmarkType::timeGetRandom;
	else if (typeStr == "meassureEntropy")
		type = benchmarkType::meassureEntropy;
	else if (typeStr == "testTimeToSeed")
		type = benchmarkType::testTimeToSeed;
	else
		type = benchmarkType::noType;

	std::string outputDir = vm["outputDir"].as<std::string>();
	bool mkdir;
	mkdir = std::filesystem::create_directory(outputDir);
	if (mkdir) {
		std::cout << "Create output directory:" << outputDir << "\n";
	}
	for (int i = 0; i < repetitions; i++) {
		std::string outputFileName =
			outputDir + typeStr + "." + std::to_string(i) + "." + vm["outputFile"].as<std::string>();
		std::cout << outputFileName << "\n";
		switch (type) {
		case benchmarkType::timeGetRandom:
		start_getRandom(optionVec, outputFileName, saveOutput);
		break;

		case benchmarkType::meassureEntropy:
		start_meassureEntropy(optionVec, outputFileName);
		break;

		case benchmarkType::testTimeToSeed:
		start_timeToSeed(optionVec, outputFileName);
		break;

		default:
		std::cout << "Unknown benchmark name:" << typeStr << "\n";
		break;
		}
	}

	return 0;
}

void call_test(const std::string testName) {
	if (testName == "testUnprivRand") {
		testUnprivRand();
	} else if (testName == "testPrivRand") {
		testPrivRand();
	} else if (testName == "testStatus") {
		testStatus();
	} else if (testName == "testSeed") {
		testSeed();
		// }else if(testName == "getSeed"){ //todo: just a test remove this block
		// later
		//   std::vector<std::pair<size_t,std::string>> v;
		//   getSeed(v);
	} else {
		std::cout << "unkown test with name:'" << testName << "' called\n";
	}
}

int main(int argc, char *argv[]) {
	namespace po = boost::program_options;

	bool help;
	std::vector<std::string> benchmarkOptions;
	int repetitions;
	std::string test;
	std::string outputFile;
	std::string outputDir;
	//todo optional
	try {
		// clang-format off
		po::options_description desc{"Options"};
		desc.add_options()
			("help,h", po::bool_switch(&help)->default_value(false), "Help screen")
			("b,benchmark", po::value<std::vector<std::string>>(&benchmarkOptions)->multitoken()->zero_tokens()->composing(), "Benchmark")
			("r,repetitions", po::value<int>(&repetitions)->default_value(1), "Repetitions of choosen Programm") //<--todo
			("t,test", po::value<std::string>(&test), "call test functions")
			("outputFile", po::value<std::string>(&outputFile)->default_value("data.bench"), "Suffix of output file. Name of output: 'benchmarkType'.'repetition'.'suffix'")
			("outputDir", po::value<std::string>(&outputDir)->default_value("./res/"), "Result directory")
			("save", po::value<bool>()->default_value(false), "Do save returned values from esdm_rpcc functions called in benchmarks: pr")
		;
		// clang-format on

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// size_t rep = vm["r"].as<size_t>();

		if (help)
			std::cout << desc << '\n';
		else if (vm.count("b"))
			call_benchmark(vm["b"].as<std::vector<std::string>>(), vm);
		else if (vm.count("t"))
			call_test(vm["t"].as<std::string>());

		} catch (const po::	error &ex) {
			std::cerr << ex.what() << '\n';
		}

	return EXIT_SUCCESS;
}
