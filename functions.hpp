#pragma once

#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <esdm/esdm_rpc_client.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "esdmRpcFuntions.hpp"
#include "writer.hpp"

using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using measureEntropyOutput = std::vector<std::pair<int64_t, int>>;

// todo: function int64_6 ->
// Json::Value[duration1:unit1,time1],[duration2:unit2,time2]

// Json::Value timeFieldFromNanoseconds(int64_t timeInNanoseconds) {
// std::cout << timeInNanoseconds << "<---input\n";
// ns nanoseconds = ns(timeInNanoseconds);
// std::cout << nanoseconds.count() << "<---chrono ns\n";
// us mikroseconds = std::chrono::duration<int64_t, ns>(nanoseconds);
// std::cout << mikroseconds.count() << "<---chrono us\n";
// us milliseconds = us(nanoseconds);
// std::cout << milliseconds.count() << "<---chrono ms\n";
// us seconds = us(nanoseconds);
// std::cout << seconds.count() << "<---chrono s\n";
// Json::Value root;
// root["duration"];
// return root;
// }

// todorm: testFunction remove
//  void testTest(int64_t testInput = 1243567890123){
//  	Json::Value root;
//  	root["duration"] = timeFieldFromNanoseconds(testInput);
//  	Writer writer;
//  	writer.writeOutputFile("testJsonOutput", root);
//  }

void getRandomNumbers(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	std::optional<std::vector<int64_t>>& durationVec,
	// std::optional<std::pair<std::vector<std::pair<size_t,
	// std::string>>,std::vector<int64_t>>>& returnVec,
	const int requestSize = 32) {
	size_t ret = 0;
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestSize);
	std::chrono::time_point<std::chrono::system_clock> beforeRpcc =
		Clock::now();
	esdm_invoke(
		esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
	std::chrono::time_point<std::chrono::system_clock> afterRpcc = Clock::now();
	assert(ret > 0);
	if (returnVec != std::nullopt && durationVec != std::nullopt) {
		std::string returnString;
		returnString.append("0x");
		for (size_t i = 0; i < randBytes.size(); ++i) {
			returnString.append(
				str(boost::format("%02x") % static_cast<int>(randBytes[i])));
		}
		returnVec->push_back(std::make_pair(ret, returnString));

		auto duration = afterRpcc - beforeRpcc;
		ns durationRpcc = std::chrono::duration_cast<ns>(duration);
		durationVec->push_back(durationRpcc.count());
		// returnVec->second.push_back(durationRpcc);
	}
}

// todo: measure rate
int64_t benchmarkTimeGetRandom(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	std::optional<std::vector<int64_t>>& durationVec,
	// std::optional<std::pair<std::vector<std::pair<size_t,
	// std::string>>,std::vector<int64_t>>>& returnVec,
	const size_t requests = 1, const int requestSize = 32) {
	esdm_rpcc_init_unpriv_service(NULL);
	std::chrono::time_point<std::chrono::system_clock> start = Clock::now();

	assert(requests >= 1);
	// nothing would speak against assert(requestSize >= 0); appart from the
	// assertion of returned values: assert(ret > 0) in getRandomNumbers()
	assert(requestSize > 0);
	for (size_t i = 0; i < requests; i++) {
		getRandomNumbers(returnVec, durationVec, requestSize);
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

	// esdm_invoke(esdm_rpcc_rnd_clear_pool());

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

bool startTimeGetRandom(const std::string outputFileName, Config config) {
	// execute 'timeGetRandom' benchmark
	std::vector<std::string> parameters = config.getBenchmarkParameters();
	bool saveRandomOutput = config.getSave();
	int64_t outputDuration = 0;
	if (parameters.size() > 2) {
		std::cout << "to many arguments for the timeGetRandom benchmark\n. "
					 "Expected at most 2 "
					 "Arguments:\n"
				  << "(1) requests\n"
				  << "(2) requestSize\n";
		return false;
	}

	// todo maybe compose rpccRetVec and individualDurationRetVec into an
	// std::optional<std::pair<....>>
	std::optional<std::vector<std::pair<size_t, std::string>>> rpccRetVec =
		std::vector<std::pair<size_t, std::string>>();
	// std::optional<std::pair<std::vector<std::pair<size_t,
	// std::string>>,std::vector<int64_t>>> rpccRetVec =
	// std::pair<std::vector<std::pair<size_t,
	// std::string>>,std::vector<int64_t>>();
	std::optional<std::vector<int64_t>> individualDurationRetVec =
		std::vector<int64_t>();
	if (!saveRandomOutput) {
		rpccRetVec = std::nullopt;
		individualDurationRetVec = std::nullopt;
	}
	if (parameters.size() == 2) {
		outputDuration = benchmarkTimeGetRandom(
			rpccRetVec, individualDurationRetVec, std::stoi(parameters[0]),
			std::stoi(parameters[1]));
	} else if (parameters.size() == 1) {
		outputDuration = benchmarkTimeGetRandom(
			rpccRetVec, individualDurationRetVec, std::stoi(parameters[0]));
	} else {
		outputDuration =
			benchmarkTimeGetRandom(rpccRetVec, individualDurationRetVec);
	}
	// todo: json field names as macros?

	// save result
	Json::Value data;
	data["info"] = "Structure of entries in the field 'output':\n"
				   "duration:total duration for all requests\n"
				   "returnedValues: (numberOfReturnedBytes, returnedBytes)\n";
	Json::Value durationEntry;
	durationEntry["unit"] = "nanoseconds";
	durationEntry["time"] = outputDuration;
	// todo: durationEntry["timeInSeconds"] = std::chrono::duration_cast<
	data["duration"] = durationEntry;

	assert(rpccRetVec->size() == individualDurationRetVec->size());
	for (size_t i = 0; i < rpccRetVec->size(); i++) {
		Json::Value returnedValues;
		// returnedValues["numberOfReturnedBytes"] = i.first.first;
		// returnedValues["returnedBytes"] = i.first.second;
		// returnedValues["invokationDuration"] = i.second;
		returnedValues["numberOfReturnedBytes"] = (*rpccRetVec)[i].first;
		returnedValues["returnedBytes"] = (*rpccRetVec)[i].second;
		returnedValues["invokationDuration"]["unit"] = "nanoseconds";
		returnedValues["invokationDuration"]["time"] =
			(*individualDurationRetVec)[i];

		data["returnedValues"].append(returnedValues);
	}

	Writer writer;
	return writer.writeOutputFile(outputFileName, data, config);
}

bool startMeasureEntropy(const std::string outputFileName, Config config) {
	// execute 'measureEntropy' benchmark
	measureEntropyOutput output;
	std::vector<std::string> parameters = config.getBenchmarkParameters();
	if (parameters.size() > 2) {
		std::cout << "To many arguments for the measureEntropy benchmark.\n "
					 "Expected at "
					 "most 2 "
					 "Arguments:\n"
				  << "(1) duration: Check the entropy level for 'duration' "
					 "milliseconds\n"
				  << "(2) delta   : If set, only check after 'delta' "
					 "microseconds have "
					 "passed\n";
		return false;
	} else if (parameters.size() == 2) {
		benchmarkMeasureEntropy(output, std::stoi(parameters[0]),
								std::stoi(parameters[1]));
	} else if (parameters.size() == 1) {
		benchmarkMeasureEntropy(output, std::stoi(parameters[0]));
	} else if (parameters.size() == 0) {
		benchmarkMeasureEntropy(output);
	}

	// save result
	Json::Value data;
	data["info"] = "Structure of entries in the field 'output':\n"
				   "Start Time\n"
				   "measurements: (timestamp, entropyLevel)\n";

	Json::Value startTime;
	startTime["unit"] = "nanoseconds";
	assert(output.size() > 0);
	if (output.size() > 0)
		startTime["time"] = output.front().first;
	output.erase(output.begin());
	data["startTime"] = startTime;

	for (auto i : output) {
		Json::Value entry;
		entry["timestamp"] = i.first;
		entry["entropyLevel"] = i.second;
		data["measurements"].append(entry);
	}

	Writer writer;
	return writer.writeOutputFile(outputFileName, data, config);
}

bool startTimeToSeed(const std::string outputFileName, Config config) {
	std::vector<std::string> parameters = config.getBenchmarkParameters();
	int64_t outputTimeToSeed;
	// execute 'timeToSeed' benchmark
	if (parameters.size() > 0) {
		std::cout << "To many arguments for the measureEntropy benchmark.\n "
					 "Expected 0 "
					 "Argument:\n"
				  << "(0) type: 'TimeToSeed'\n";
		return false;
	} else if (parameters.size() == 0) {
		outputTimeToSeed = timeTillSeeded();
	}
	// save result
	Json::Value data;
	data["info"] = "Structure of entries in the field 'output':\n"
				   "Time until ESDM was Fully Seeded\n"
				   "measurement: (timeUntilSeeded) \n";

	data["measurement"]["unit"] = "nanoseconds";
	data["measurement"]["time"] = outputTimeToSeed;

	Writer writer;
	return writer.writeOutputFile(outputFileName, data, config);
}

const std::string availableBenchmarkFunctions =
	// clang-format off
				  "Available benchmarks: \n"
				  "timeGetRandom : Call esdm_rpcc_get_random_bytes_pr\n"
				  "\t -Parameters:\n"
				  "\t\t requests :\t Number of times the function is called\n"
				  "\t\t requestSize:\tAmount of requested random bytes\n"
				  "measureEntropy : Read the entropy status of the esdm\n"
				  "\t -Parameters:\n"
				  "\t\t observationTime:\tFor how long the entropy status is measured."
			   	  "(default: until fully seeded)\n"
				  "\t\t deltaMs:\tDelta until next measurement is attempted (default: 0ms)"
				  "timeToSeed : Measure the time till esdm is fully seeded\n";
// clang-format on
;

std::string createOutFileName(Config config, int runningNumberRepetition) {
	std::string typeStr;
	if (config.getFunctionType() == FunctionType::benchmark)
		typeStr = benchmarkTypeToString(config.getBenchmarkType());
	else if (config.getFunctionType() == FunctionType::test)
		typeStr = functionTypeToString(config.getFunctionType());
	std::string outputFileName = config.getOutputDirName() + typeStr + "." +
								 std::to_string(runningNumberRepetition) + "." +
								 config.getOutputFileName();
	return outputFileName;
}

bool callBenchmark(Config config) {
	BenchmarkType type = config.getBenchmarkType();
	if (config.getBenchmarkType() == BenchmarkType::noType) {
		std::cout << "please specify a benchmark to run:\n"
				  << availableBenchmarkFunctions;
		return false;
	}

	std::string typeStr = benchmarkTypeToString(type);
	if (type == BenchmarkType::unknownType) {
		std::cout << "Unknown benchmark: " << config.getRawBenchmarkType()
				  << "\n";
		return false;
	}

	const std::string outputDir = config.getOutputDirName();
	bool mkdir;
	mkdir = std::filesystem::create_directory(outputDir);
	if (mkdir) {
		std::cout << "Create output directory:" << outputDir << "\n";
	}

	bool successfullExecution = true;
	const int repetitions = config.getRepetitions();
	assert(repetitions > 0);
	for (int i = 0; i < repetitions; i++) {
		std::string outputFileName = createOutFileName(config, i);
		switch (type) {
		case BenchmarkType::timeGetRandom:
			successfullExecution &= startTimeGetRandom(outputFileName, config);
			break;

		case BenchmarkType::measureEntropy:
			successfullExecution &= startMeasureEntropy(outputFileName, config);
			break;

		case BenchmarkType::timeToSeed:
			successfullExecution &= startTimeToSeed(outputFileName, config);

		default:
			break;
		}
	}

	return successfullExecution;
}

const std::string availableTestFunctions =
	"Available test functions:\n"
	"testRandBytes : call esdm_rpcc_get_random_bytes\n"
	"testRandBytesFull : call esdm_rpcc_get_random_bytes_full\n"
	"testRandBytesMin : call esdm_rpcc_get_random_bytes_min\n"
	"testRandBytesPr : call esdm_rpcc_get_random_bytes_pr\n"
	"testWriteData : call esdm_rpcc_write_data\n"
	"testStatus     : call esdm_rpcc_status\n"
	"testSeed       : call esdm_rpcc_get_seed\n"
	"testEntCnt : call esdm_rpcc_get_ent_cnt\n"
	"testGetPoolsize : call esdm_rpcc_poolsize\n"
	"testGetWriteWakeupThresh     : call esdm_rpcc_get_write_wakeup_thresh\n"
	"testPrivSetWriteWakeupThresh : call esdm_rpcc_set_write_wakeup_thresh\n"
	"testGetMinReseedSecs : call esdm_rpcc_get_min_reseed_secs\n"
	"testPrivSetMinReseedSecs : call esdm_rpcc_set_min_reseed_secs\n"
	"testPrivAddEntropy   : call esdm_rpcc_rnd_add_entropy\n"
	"testPrivAddToEntCnt : call esdm_rpcc_rnd_add_to_ent_cnt\n"
	"testPrivClearPool : call esdm_rpcc_rnd_clear_pool\n"
	"testPrivReseedCrng : call esdm_rpcc_rnd_reseed_crng\n";

bool callTest(Config config) {
	TestType testType = config.getTestType();
	switch (testType) {
	case TestType::testRandBytes:
		testRandBytes();
		return true;
	case TestType::testRandBytesFull:
		testRandBytesFull();
		return true;
	case TestType::testRandBytesMin:
		testRandBytesMin();
		return true;
	case TestType::testRandBytesPr:
		testRandBytesPr();
		return true;
	case TestType::testWriteData:
		testWriteData();
		return true;
	case TestType::testStatus:
		testStatus();
		return true;
	case TestType::testSeed:
		testSeed();
		return true;
	case TestType::testEntCnt:
		testEntCnt();
		return true;
	case TestType::testGetPoolsize:
		testGetPoolsize();
		return true;
	case TestType::testGetWriteWakeupThresh:
		testGetWriteWakeupThresh();
		return true;
	case TestType::testPrivSetWriteWakeupThresh:
		testPrivSetWriteWakeupThresh();
		return true;
	case TestType::testGetMinReseedSecs:
		testGetMinReseedSecs();
		return true;
	case TestType::testPrivSetMinReseedSecs:
		testPrivSetMinReseedSecs();
		return true;
	case TestType::testPrivAddEntropy:
		testPrivAddEntropy();
		return true;
	case TestType::testPrivAddToEntCnt:
		testPrivAddToEntCnt();
		return true;
	case TestType::testPrivClearPool:
		testPrivClearPool();
		return true;
	case TestType::testPrivReseedCrng:
		testPrivReseedCrng();
		return true;
	case TestType::unknownType:
		std::cout << "unknown test with name:'" << config.getRawTestType()
				  << "' called\n"
				  << availableTestFunctions;
		return false;
	case TestType::noType:
		std::cout << availableTestFunctions;
		return false;
	default:
		break;
	}
	return false;
}

void createConfigFromArgs(int argc, char* argv[], Config& config) {
	namespace po = boost::program_options;
	bool help;
	bool showStatus;
	std::vector<std::string> benchmarkOptions;
	int repetitions;
	std::string test;
	std::string outputFile;
	std::string outputDir;
	bool save;

	try {
		// clang-format off
		po::options_description desc{"Options"};
		desc.add_options()
			("help,h", po::bool_switch(&help)->default_value(false), "Help screen")
			("benchmark,b", po::value<std::vector<std::string>>(&benchmarkOptions)->multitoken()->zero_tokens()->composing(), "Benchmark")
			("repetitions,r", po::value<int>(&repetitions)->default_value(1), "Repetitions of choosen benchmark")
			("test,t", po::value<std::string>(&test), "call test functions")
			("Status,S", po::bool_switch(&showStatus)->default_value(false), "show ESDM status")
			("outputFile,o", po::value<std::string>(&outputFile)->default_value("data.bench"), "Suffix of output file. Name of output: 'benchmarkType'.'repetition'.'suffix'")
			("outputDir,d", po::value<std::string>(&outputDir)->default_value("./res/"), "Result directory")
			("save,s", po::bool_switch(&save)->default_value(false), "Save output of the rpc calls made by benchmark: timeGetRandom")
		;
		// clang-format on

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		FunctionType functionType;
		BenchmarkType benchmarkType;
		std::string rawBenchmarkType;
		TestType testType;
		std::vector<std::string> benchmarkParameters;
		if (vm.count("benchmark")) {
			functionType = FunctionType::benchmark;
			testType = TestType::noType;
			if (benchmarkOptions.size() > 0) {
				rawBenchmarkType = benchmarkOptions[0];
				benchmarkType = stringToBenchmarkType(rawBenchmarkType);
				benchmarkParameters = benchmarkOptions;
				benchmarkParameters.erase(benchmarkParameters.begin());
			} else
				benchmarkType = BenchmarkType::noType;
		} else if (vm.count("test")) {
			functionType = FunctionType::test;
			benchmarkType = BenchmarkType::noType;
			testType = stringToTestType(test);
		} else if (help) {
			std::cout << desc << "\n";
		}
		config = Config(functionType, testType, benchmarkType,
						benchmarkParameters, repetitions, outputFile, outputDir,
						help, showStatus, save, test, rawBenchmarkType);
		config.printConfig(true);
		return;
	} catch (const po::error& ex) {
		std::cerr << ex.what() << '\n';
	}
	return;
}