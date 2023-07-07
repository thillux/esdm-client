#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <esdm/esdm_rpc_client.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class benchmarkType { timeGetRandom, measureEntropy, timeToSeed, noType };

using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using measureEntropyOutput = std::vector<std::pair<int64_t, int>>;

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
	esdm_rpcc_init_unpriv_service(NULL);

	size_t ret = 0;

	std::array<uint64_t, 1024 / sizeof(uint64_t)> randBytes;
	esdm_invoke(esdm_rpcc_get_seed(reinterpret_cast<uint8_t*>(randBytes.data()),
								   randBytes.size() * sizeof(uint64_t), 0));
	std::cout << "Ret: " << ret << std::endl;

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

void getRandomNumbers(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	const int requestSize = 32) {
	size_t ret = 0;
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestSize);
	esdm_invoke(
		esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
	assert(ret > 0);
	if (returnVec != std::nullopt) {
		std::string returnString;
		returnString.append("0x");
		for (size_t i = 0; i < randBytes.size(); ++i) {
			returnString.append(
				str(boost::format("%02x") % static_cast<int>(randBytes[i])));
		}
		returnVec->push_back(std::make_pair(ret, returnString));
	}
}

int64_t benchmarkTimeGetRandom(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	const size_t requests = 1, const int requestSize = 32) {
	esdm_rpcc_init_unpriv_service(NULL);
	std::chrono::time_point<std::chrono::system_clock> start = Clock::now();
	for (size_t i = 0; i < requests; i++) {
		getRandomNumbers(returnVec, requestSize);
	}
	std::chrono::time_point<std::chrono::system_clock> end = Clock::now();

	auto duration = end - start;
	ns durationNs = std::chrono::duration_cast<ns>(duration);

	esdm_rpcc_fini_unpriv_service();
	return durationNs.count();
}

void benchmarkMeasureEntropy(measureEntropyOutput& returnVec,
							 const long observationTime = 0,
							 const long deltaMs = 0) {
	assert(observationTime >= 0);
	assert(deltaMs >= 0);
	size_t ret = 0;
	esdm_rpcc_init_unpriv_service(NULL);
	char buffer[2048];
	assert(ret == 0);
	bool fullySeeded = false;
	bool firstLoop = true;
	bool useDelta = deltaMs != 0 ? true : false;
	ns maxDuration(0);
	if (observationTime > 0)
		maxDuration = std::chrono::duration_cast<ns>(ms(observationTime));
	ns duration = ns(0);
	std::chrono::time_point<std::chrono::system_clock, ns> start = Clock::now();
	std::chrono::time_point<std::chrono::system_clock, ns> now = Clock::now();
	ns delta;
	if (useDelta)
		delta = std::chrono::duration_cast<ns>(us(deltaMs));
	std::chrono::time_point<std::chrono::system_clock, ns> deltaTimePoint =
		Clock::now();

	// save start date as the first tuple in our retVec: (startTime, -1)
	int64_t startNano = start.time_since_epoch().count();
	returnVec.push_back(std::make_pair(startNano, -1));

	// if observationTime is set to the default 0 then duration == maxDuration
	//-> the loop will repeat till the esdm status says it is fully seeded
	while (!fullySeeded || (duration < maxDuration)) {
		deltaTimePoint = Clock::now();
		if (std::chrono::duration_cast<ns>(deltaTimePoint - now) < delta &&
			!firstLoop && useDelta) {
			continue;
		}
		now = Clock::now();

		/*
				todo: maybe call a function here to manipulate the entropy
		   level, if possible. (like reset, decrease the entropy) maybe
		   get_raw_entropy?
		*/

		// unsigned int entCnt;
		// esdm_invoke(esdm_rpcc_rnd_get_ent_cnt(&entCnt));
		// std::cout << "entCnt:" << entCnt << "\n";

		esdm_invoke(esdm_rpcc_status(buffer, sizeof(buffer)));
		std::string bufferString(buffer);
		std::memset(buffer, 0, sizeof(buffer));

		duration = std::chrono::duration_cast<ns>(now - start);
		// determine if esdm is fully seeded trough parsing the returned buffer
		const std::string fullySeededString = "ESDM fully seeded: true";
		fullySeeded = bufferString.find(fullySeededString) != std::string::npos;
		// get the current entropy level through string parsing
		const std::string entropyLevelString = "entropy level: ";
		size_t positionStart = bufferString.find(entropyLevelString);
		if (positionStart != std::string::npos)
			positionStart += entropyLevelString.size();

		size_t positionEnd = bufferString.find("\n", positionStart);
		assert(positionEnd != std::string::npos);
		std::optional<int> entropyLevel = 0;
		if (positionEnd != std::string::npos) {
			std::string out =
				bufferString.substr(positionStart, positionEnd - positionStart);
			entropyLevel = std::stoi(out);
		} else {
			entropyLevel = std::nullopt;
		}
		int64_t nowNano = now.time_since_epoch().count();

		std::pair<int64_t, int> entry = std::make_pair(nowNano, *entropyLevel);
		returnVec.push_back(entry);

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

	// std::array<uint8_t, 64> randomOutputBuffer;
	// esdm_invoke(esdm_rpcc_get_random_bytes_pr(randomOutputBuffer.data(),
	// 										  randomOutputBuffer.size()));

	bool fullySeeded = false;
	// bool firstLoop = true;
	while (!fullySeeded) {
		std::memset(buffer, 0, sizeof(buffer));
		esdm_invoke(esdm_rpcc_status(buffer, sizeof(buffer)));
		std::string bufferString(buffer);
		const std::string fullySeededString = "ESDM fully seeded: true";
		fullySeeded = bufferString.find(fullySeededString) != std::string::npos;

		// if (fullySeeded && firstLoop) {
		// 	firstLoop = false;
		// 	break;
		// }

		if (fullySeeded)
			end = Clock::now();
	}

	ns duration = end - start;

	esdm_rpcc_fini_unpriv_service();
	return duration.count();
}

bool startTimeGetRandom(const std::vector<std::string>& optionVec,
						const std::string outputFileName,
						const bool saveRandomOutput = false) {
	// execute 'timeGetRandom' benchmark
	int64_t outputDuration = 0;
	assert(optionVec.size() >= 1);
	if (optionVec.size() > 3) {
		std::cout << "to many arguments for the timeGetRandom benchmark\n. "
					 "Expected at most 3 "
					 "Arguments:\n"
				  << "(0) type: 'timeGetRandom'\n"
				  << "(1) requests\n"
				  << "(2) requestSize\n";
		return false;
	}

	std::optional<std::vector<std::pair<size_t, std::string>>> retVec =
		std::vector<std::pair<size_t, std::string>>();
	if (!saveRandomOutput) {
		retVec = std::nullopt;
	}

	if (optionVec.size() == 3) {
		outputDuration = benchmarkTimeGetRandom(retVec, std::stoi(optionVec[1]),
												std::stoi(optionVec[2]));
	} else if (optionVec.size() == 2) {
		outputDuration =
			benchmarkTimeGetRandom(retVec, std::stoi(optionVec[1]));
	} else {
		outputDuration = benchmarkTimeGetRandom(retVec);
	}
	// save result
	std::fstream outputFile{outputFileName, outputFile.out};
	if (!outputFile.is_open()) {
		std::cout << "Could not open output file.\n";
		return false;
	}
	outputFile << "duration:\t" << outputDuration << "\n";
	for (auto i : *retVec) {
		outputFile << i.first << "\t" << i.second << "\n";
	}
	outputFile.close();
	if (outputFile.bad()) {
		std::cout << "Error while writing to file:" << outputFileName << "\n";
		return false;
	}
	return true;
}

bool startMeasureEntropy(const std::vector<std::string>& optionVec,
						 std::string outputFileName) {
	// execute 'measureEntropy' benchmark
	measureEntropyOutput output;
	assert(optionVec.size() >= 1);
	if (optionVec.size() > 3) {
		std::cout << "To many arguments for the measureEntropy benchmark.\n "
					 "Expected at "
					 "most 3 "
					 "Arguments:\n"
				  << "(0) type    : 'measureEntropy'\n"
				  << "(1) duration: Check the entropy level for 'duration' "
					 "milliseconds\n"
				  << "(2) delta   : If set, only check after 'delta' "
					 "microseconds have "
					 "passed\n";
		return -1;
	} else if (optionVec.size() == 3) {
		benchmarkMeasureEntropy(output, std::stoi(optionVec[1]),
								std::stoi(optionVec[2]));
	} else if (optionVec.size() == 2) {
		benchmarkMeasureEntropy(output, std::stoi(optionVec[1]));
	} else if (optionVec.size() == 1) {
		benchmarkMeasureEntropy(output);
	}
	// save result
	std::fstream outputFile{outputFileName, outputFile.out};
	if (!outputFile.is_open()) {
		std::cout << "Could not open output file.\n";
		return false;
	}
	for (auto i : output) {
		outputFile << i.first << "\t" << i.second << "\n";
		// fprintf(f, "%lu\t%d\n", i.first, i.second);
	}
	outputFile.close();
	if (outputFile.bad()) {
		std::cout << "Error while writing to file:" << outputFileName << "\n";
		return false;
	}
	return true;
}

bool startTimeToSeed(const std::vector<std::string>& optionVec,
					 std::string outputFileName) {
	int64_t outputTimeToSeed;
	// execute 'timeToSeed' benchmark
	assert(optionVec.size() >= 1);
	if (optionVec.size() > 1) {
		std::cout << "To many arguments for the measureEntropy benchmark.\n "
					 "Expected at most 1 "
					 "Argument:\n"
				  << "(0) type: 'TimeToSeed'\n";
	} else if (optionVec.size() == 1) {
		outputTimeToSeed = timeTillSeeded();
	}
	// save result
	std::fstream outputFile{outputFileName, outputFile.out};
	if (!outputFile.is_open()) {
		std::cout << "Could not open output file.\n";
		return false;
	}
	outputFile << outputTimeToSeed << "\n";
	outputFile.close();
	if (outputFile.bad()) {
		std::cout << "Error while writing to file:" << outputFileName << "\n";
		return false;
	}
	return true;
}

bool callBenchmark(const std::vector<std::string>& optionVec,
				   const boost::program_options::variables_map& vm) {
	std::cout << "vector.size():" << optionVec.size() << "\n";

	if (optionVec.size() == 0) {
		std::cout
			<< "please specify a benchmark to run:\n"
			<< "'timeGetRandom(requests,requestSize)':\tCall "
			   "esdm_rpcc_get_random_bytes_pr\n"
			<< "\t requests       :\tNumber of times the function is called\n"
			<< "\t requestSize    :\tAmount of requested random bytes\n\n"
			<< "'measureEntropy(observationTime,deltaMs)':\t Read the entropy "
			   "status of the esdm\n"
			<< "\t observationTime:\tFor how long the entropy status is "
			   "measured "
			   "(default: "
			   "until fully seeded)\n"
			<< "\t deltaMs        :\tOnly read the entropy status after "
			   "'deltaMs' "
			   "milliseconds (default: 0) have passed since the last "
			   "observationTime\n\n"
			<< "'timeToSeed()':\tMeasure the time till esdm is fully "
			   "seeded\n";
		return false;
	}

	benchmarkType type = benchmarkType::noType;
	std::string typeStr = optionVec[0];
	if (typeStr == "timeGetRandom")
		type = benchmarkType::timeGetRandom;
	else if (typeStr == "measureEntropy")
		type = benchmarkType::measureEntropy;
	else if (typeStr == "timeToSeed")
		type = benchmarkType::timeToSeed;

	std::string outputDir = vm["outputDir"].as<std::string>();
	bool mkdir;
	mkdir = std::filesystem::create_directory(outputDir);
	if (mkdir) {
		std::cout << "Create output directory:" << outputDir << "\n";
	}

	if (type == benchmarkType::noType) {
		std::cout << "Unknown benchmark name \"" << typeStr << "\"\n"
				  << "Available benchmarks: \n"
				  << "timeGetRandom\n"
				  << "measureEntropy\n"
				  << "timeToSeed\n";
		return false;
	}

	bool successfullExecution = true;
	bool saveOutput = vm["save"].as<bool>();
	int repetitions = vm["r"].as<int>();
	assert(repetitions > 0);
	for (int i = 0; i < repetitions; i++) {
		std::string outputFileName = outputDir + typeStr + "." +
									 std::to_string(i) + "." +
									 vm["outputFile"].as<std::string>();
		std::cout << outputFileName << "\n";
		switch (type) {
		case benchmarkType::timeGetRandom:
			successfullExecution &=
				startTimeGetRandom(optionVec, outputFileName, saveOutput);
			break;

		case benchmarkType::measureEntropy:
			successfullExecution &=
				startMeasureEntropy(optionVec, outputFileName);
			break;

		case benchmarkType::timeToSeed:
			successfullExecution &= startTimeToSeed(optionVec, outputFileName);

		default:
			break;
		}
	}

	return successfullExecution;
}

bool callTest(const std::string testName) {
	if (testName == "testUnprivRand") {
		testUnprivRand();
	} else if (testName == "testPrivRand") {
		testPrivRand();
	} else if (testName == "testStatus") {
		testStatus();
	} else if (testName == "testSeed") {
		testSeed();
	} else if (testName == "h" || testName == "help") {
		std::cout << "Available test functions:\n"
				  << "testUnprivRand : call esdm_rpcc_get_random_bytes_pr\n"
				  << "testPrivRand   : call esdm_rpcc_rnd_add_entropy\n"
				  << "testStatus     : call esdm_rpcc_status\n"
				  << "testSeed       : call esdm_rpcc_get_seed\n";
	} else {
		std::cout << "unkown test with name:'" << testName << "' called\n";
		return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	namespace po = boost::program_options;

	bool help;
	std::vector<std::string> benchmarkOptions;
	int repetitions;
	std::string test;
	std::string outputFile;
	std::string outputDir;
	try {
		// clang-format off
		po::options_description desc{"Options"};
		desc.add_options()
			("help,h", po::bool_switch(&help)->default_value(false), "Help screen")
			("b,benchmark", po::value<std::vector<std::string>>(&benchmarkOptions)->multitoken()->zero_tokens()->composing(), "Benchmark")
			("r,repetitions", po::value<int>(&repetitions)->default_value(1), "Repetitions of choosen benchmark")
			("t,test", po::value<std::string>(&test), "call test functions")
			("outputFile", po::value<std::string>(&outputFile)->default_value("data.bench"), "Suffix of output file. Name of output: 'benchmarkType'.'repetition'.'suffix'")
			("outputDir", po::value<std::string>(&outputDir)->default_value("./res/"), "Result directory")
			("save", po::value<bool>()->default_value(false), "Save output of the rpc calls made by benchmark: timeGetRandom")
		;
		// clang-format on

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// size_t rep = vm["r"].as<size_t>();

		bool exit = false;

		if (help)
			std::cout << desc << '\n';
		else if (vm.count("b"))
			exit = callBenchmark(vm["b"].as<std::vector<std::string>>(), vm);
		else if (vm.count("t"))
			exit = callTest(vm["t"].as<std::string>());
		else {
			std::cout << desc << '\n';
			exit = false;
		}
		if (exit)
			return EXIT_SUCCESS;

	} catch (const po::error& ex) {
		std::cerr << ex.what() << '\n';
	}

	return EXIT_FAILURE;
}
