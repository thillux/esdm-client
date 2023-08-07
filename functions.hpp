#pragma once

#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <esdm/esdm_rpc_client.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "esdmRpcFuntions.hpp"
#include "writer.hpp"

// // todorm: testFunction remove
// void testTest(int64_t testInput = 1243567890123) {
// 	Json::Value root;
// 	timeFieldFromNanoseconds(testInput, root, "ns", "otherData");
// 	timeFieldFromNanoseconds(testInput, root, "us", "otherData");
// 	timeFieldFromNanoseconds(testInput, root, "ms", "otherData");
// 	timeFieldFromNanoseconds(testInput, root, "s", "otherData");
// 	Writer writer;
// 	writer.writeOutputFile("testJsonOutput", root);
// }

void getRandomNumbers(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	std::optional<std::vector<int64_t>>& durationVec,
	// std::optional<std::pair<std::vector<std::pair<size_t,
	// std::string>>,std::vector<int64_t>>>& returnVec,
	const int requestSize = 32, const std::string rpccFunction = "pr") {
	size_t ret = 0;
	std::vector<uint8_t> randBytes;
	randBytes.resize(requestSize);
	bool wantFunctionRandomBytesPr = rpccFunction == "pr";
	assert(rpccFunction == "pr" || rpccFunction == "full");
	std::chrono::time_point<std::chrono::system_clock> beforeRpcc =
		Clock::now();
	if (wantFunctionRandomBytesPr)
		esdm_invoke(
			esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
	else
		esdm_invoke(esdm_rpcc_get_random_bytes_full(randBytes.data(),
													randBytes.size()));
	std::chrono::time_point<std::chrono::system_clock> afterRpcc = Clock::now();
	assert(ret > 0);
	// std::string testFile = "testFile";
	// std::ofstream os;
	// os.open(testFile, std::ios::binary|std::ios::app);
	// os.write(reinterpret_cast<const char*>(randBytes.data()), randBytes.size());
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
	}
}

int64_t benchmarkTimeGetRandom(
	std::optional<std::vector<std::pair<size_t, std::string>>>& returnVec,
	std::optional<std::vector<int64_t>>& durationVec,
	const size_t requests = 1, const int requestSize = 32,
	const std::string rpccFunction = "pr") {
	esdm_rpcc_init_unpriv_service(NULL);
	std::chrono::time_point<std::chrono::system_clock> start = Clock::now();

	assert(requests >= 1);
	// nothing would speak against assert(requestSize >= 0); appart from the
	// assertion of returned values: assert(ret > 0) in getRandomNumbers()
	assert(requestSize > 0);
	for (size_t i = 0; i < requests; i++) {
		getRandomNumbers(returnVec, durationVec, requestSize, rpccFunction);
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
	//todo first loop

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
	if (parameters.size() > 3) {
		std::cout << "to many arguments for the timeGetRandom benchmark\n. "
					 "Expected at most 3 "
					 "Arguments:\n"
				  << "(1) requests\n"
				  << "(2) requestSize\n"
				  << "(3) rpccFunction: 'pr' or 'full'\n";
		return false;
	}

	std::string rpccFunction = "pr";
	std::optional<std::vector<std::pair<size_t, std::string>>> rpccRetVec =
		std::vector<std::pair<size_t, std::string>>();
	std::optional<std::vector<int64_t>> individualDurationRetVec =
		std::vector<int64_t>();
	if (!saveRandomOutput) {
		rpccRetVec = std::nullopt;
		individualDurationRetVec = std::nullopt;
	}
	if (parameters.size() == 3) {
		rpccFunction = parameters[2];
		if (rpccFunction != "pr" && rpccFunction != "full")
			return false;
		outputDuration = benchmarkTimeGetRandom(
			rpccRetVec, individualDurationRetVec, std::stoi(parameters[0]),
			std::stoi(parameters[1]), rpccFunction);
	} else if (parameters.size() == 2) {
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
	data["info"] =
		"Structure of entries in the field 'output':\n"
		"duration:total duration for all requests\n"
		"returnedValues: (numberOfReturnedBytes, returnedBytes)\n"
		"rate: bytes per seconds (uses the total duration and "
		"totalBytesReturned\n"
		"esdm_rpcc_function: esdm_rpcc function name invoked in the benchmark";
	timeFieldFromNanoseconds(outputDuration, data, "ns", "duration");
	timeFieldFromNanoseconds(outputDuration, data, "ms", "duration");

	int64_t totalBytesReturned = 0;
	assert(rpccRetVec->size() == individualDurationRetVec->size());
	for (size_t i = 0; i < rpccRetVec->size(); i++) {
		Json::Value returnedValues;
		int numberOfReturnedBytes = (*rpccRetVec)[i].first;
		returnedValues["numberOfReturnedBytes"] = numberOfReturnedBytes;
		totalBytesReturned += numberOfReturnedBytes;
		returnedValues["returnedBytes"] = (*rpccRetVec)[i].second;
		int64_t individualDuration = (*individualDurationRetVec)[i];
		timeFieldFromNanoseconds(individualDuration, returnedValues, "ns",
								 "invokationDuration");
		timeFieldFromNanoseconds(individualDuration, returnedValues, "ms",
								 "invokationDuration");
		data["returnedValues"].append(returnedValues);
	}
	data["totalBytesReturned"] = totalBytesReturned;

	data["rate"]["bytesPerSeconds"] =
		totalBytesReturned * (((double)(1000000000) / outputDuration));
	data["esdm_rpcc_function"] = rpccFunction == "pr"
									 ? "esdm_rpcc_get_random_bytes_pr"
									 : "esdm_rpcc_get_random_bytes_full";
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

	timeFieldFromNanoseconds(outputTimeToSeed, data, "ns", "measurement");
	timeFieldFromNanoseconds(outputTimeToSeed, data, "ms", "measurement");

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
				  "\t\t rpccFunction:\t whether to use esdm_rpcc_get_random_bytes_pr or ..._random_bytes_full"
				  "\t\t\t as the benchmarked function\n"
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
	std::string outputFileName = config.getOutputDirName() + "/" + typeStr + "." +
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
	makeOutputDir(outputDir);
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
	// clang-format off
	"Available test functions:\n"
	"FUNCTION_NAME        MAX_NUMBER_OF_ARGS                 DESCRIPTION\n"
	"testRandBytes                (2) size_t outfileName : call esdm_rpcc_get_random_bytes\n"
	"testRandBytesFull            (2) size_t outfileName : call esdm_rpcc_get_random_bytes_full\n"
	"testRandBytesMin             (2) size_t outfileName : call esdm_rpcc_get_random_bytes_min\n"
	"testRandBytesPr              (2) size_t outfileName : call esdm_rpcc_get_random_bytes_pr\n"
	"testWriteData                (1) std::string        : call esdm_rpcc_write_data\n"
	"testStatus                   (0)                    : call esdm_rpcc_status\n"
	"testSeed                     (0)                    : call esdm_rpcc_get_seed\n"
	"testEntCnt                   (0)                    : call esdm_rpcc_get_ent_cnt\n"
	"testPrivAddEntropy           (1) std::string        : call esdm_rpcc_rnd_add_entropy\n"
	"testPrivAddToEntCnt          (1) unsigned int       : call esdm_rpcc_rnd_add_to_ent_cnt\n"
	"testPrivClearPool            (0)                    : call esdm_rpcc_rnd_clear_pool\n"
	"testPrivReseedCrng           (0)                    : call esdm_rpcc_rnd_reseed_crng\n"
	"testGetPoolsize              (0)                    : call esdm_rpcc_poolsize\n"
	"testGetWriteWakeupThresh     (0)                    : call esdm_rpcc_get_write_wakeup_thresh\n"
	"testPrivSetWriteWakeupThresh (1) unsigned int       : call esdm_rpcc_set_write_wakeup_thresh\n"
	"testGetMinReseedSecs         (0)                    : call esdm_rpcc_get_min_reseed_secs\n"
	"testPrivSetMinReseedSecs     (1) unsigned int       : call esdm_rpcc_set_min_reseed_secs\n"
	"testJentKernel               (1) unsidned int       : call kcapi_rng_generate\n";
// clang-format on

int testMaximalExpectedParameters(TestType type) {
	// clang-format off
	//if at most 1 parameter
	if(
	type == TestType::testRandBytes ||
	type == TestType::testRandBytesFull ||
	type == TestType::testRandBytesMin ||
	type == TestType::testRandBytesPr
	)
		return 2;
	else if(
	type == TestType::testWriteData ||
	type == TestType::testPrivAddEntropy ||
	type == TestType::testPrivAddToEntCnt ||
	type == TestType::testPrivSetWriteWakeupThresh ||
	type == TestType::testPrivSetMinReseedSecs
#ifdef JENT_KERNEL
	 || type == TestType::testJentKernel
#endif
	)
		return 1;
	//at most 0 parameters
	else if(
	type == TestType::testStatus ||
	type == TestType::testSeed ||
	type == TestType::testEntCnt ||
	type == TestType::testPrivClearPool ||
	type == TestType::testPrivReseedCrng ||
	type == TestType::testGetPoolsize ||
	type == TestType::testGetWriteWakeupThresh ||
	type == TestType::testGetMinReseedSecs)
		return 0;
	else{
		assert(type == TestType::noType || type == TestType::unknownType);
		return 0;
	}
	//clang-format on
}

bool callTest(Config config) {
	TestType testType = config.getTestType();
	size_t maximalParametersForTest = testMaximalExpectedParameters(testType);
	if(maximalParametersForTest < config.getTestParameters().size() && testType != TestType::noType && testType != TestType::unknownType){
		std::cout << "Got " << config.getTestParameters().size() << " parameter(s): ";
		config.printStringVector(config.getTestParameters());
		std::cout << "Expected (at most) " << maximalParametersForTest
		<< "for test '" << testTypeToString(testType)
		<< "'.\n";
		return false;
	};

	bool additionalTestParameters = config.getTestParameters().size() != 0;
	size_t additionalTestParametersCount = config.getTestParameters().size();
	std::string firstParameterString;
	std::string secondParameterString;
	int firstParameterInt = 0;
	std::string thirdParameterString;
	int repetitions = config.getRepetitions();
	bool moreRepetitions = false;
	if(additionalTestParameters){
		if(testType == TestType::testWriteData || testType == TestType::testPrivAddEntropy)
			firstParameterString = config.getTestParameters()[0];
		else
			firstParameterInt = stoi(config.getTestParameters()[0]);
			if(additionalTestParametersCount >= 2)
				secondParameterString = config.getTestParameters()[1];
			if(repetitions != 1)
				moreRepetitions = true;
	}
		
	switch (testType) {
	case TestType::testRandBytes:
		if(additionalTestParameters)
			if(moreRepetitions)
				testRandBytes(firstParameterInt, secondParameterString, repetitions);
			else if(additionalTestParametersCount == 2)
				testRandBytes(firstParameterInt, secondParameterString);
			else
				testRandBytes(firstParameterInt);
		else
			testRandBytes();
		return true;
	case TestType::testRandBytesFull:
		if(additionalTestParameters)
			if(moreRepetitions)
				testRandBytesFull(firstParameterInt, secondParameterString, repetitions);
			else if(additionalTestParametersCount == 2)
				testRandBytesFull(firstParameterInt, secondParameterString);
			else
				testRandBytesFull(firstParameterInt);
		else
			testRandBytesFull();
		return true;
	case TestType::testRandBytesMin:
		if(additionalTestParameters)
			if(moreRepetitions)
				testRandBytesMin(firstParameterInt, secondParameterString, repetitions);
			else if(additionalTestParametersCount == 2)
				testRandBytesMin(firstParameterInt, secondParameterString);
			else
				testRandBytesMin(firstParameterInt);
		else
			testRandBytesMin();
		return true;
	case TestType::testRandBytesPr:
		if(additionalTestParameters)
			if(moreRepetitions)
				testRandBytesPr(firstParameterInt, secondParameterString, repetitions);
			else if(additionalTestParametersCount == 2)
				testRandBytesPr(firstParameterInt, secondParameterString);
			else
				testRandBytesPr(firstParameterInt);
		else
			testRandBytesPr();
		return true;
	case TestType::testWriteData:
		if(additionalTestParameters)
			testWriteData(firstParameterString);
		else
			testWriteData();
		return true;
	case TestType::testPrivAddEntropy:
		if(additionalTestParameters){
			testPrivAddEntropy(firstParameterString);
		}
		else
			testPrivAddEntropy();
		return true;
	case TestType::testPrivAddToEntCnt:
		if(additionalTestParameters)
			testPrivAddToEntCnt(firstParameterInt);
		else
			testPrivAddToEntCnt();
		return true;
	case TestType::testPrivSetWriteWakeupThresh:
		if(additionalTestParameters)
			testPrivSetWriteWakeupThresh(firstParameterInt);
		else
			testPrivSetWriteWakeupThresh();
		return true;
	case TestType::testGetMinReseedSecs:
		testGetMinReseedSecs();
		return true;
	case TestType::testPrivSetMinReseedSecs:
		if(additionalTestParameters)
			testPrivSetMinReseedSecs(firstParameterInt);
		else
			testPrivSetMinReseedSecs();
		return true;
#ifdef JENT_KERNEL
	case TestType::testJentKernel:
		if(additionalTestParameters)
			kcapiTest(firstParameterInt);
		else
			kcapiTest();
		return true;
#endif
	case TestType::testStatus:
		testStatus();
		return true;
	case TestType::testSeed:
		testSeed();
		return true;
	case TestType::testEntCnt:
		testEntCnt();
		return true;
	case TestType::testPrivClearPool:
		testPrivClearPool();
		return true;
	case TestType::testPrivReseedCrng:
		testPrivReseedCrng();
		return true;
	case TestType::testGetPoolsize:
		testGetPoolsize();
		return true;
	case TestType::testGetWriteWakeupThresh:
		testGetWriteWakeupThresh();
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
	std::vector<std::string> test;
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
			("test,t", po::value<std::vector<std::string>>(&test)->multitoken()->zero_tokens()->composing(), "call test functions")
			("Status,S", po::bool_switch(&showStatus)->default_value(false), "show ESDM status")
			("outputFile,o", po::value<std::string>(&outputFile)->default_value("data.bench"), "Suffix of output file. Name of output: 'benchmarkType'.'repetition'.'suffix'")
			("outputDir,d", po::value<std::string>(&outputDir)->default_value("./res/"), "Result directory")
			("save,s", po::bool_switch(&save)->default_value(false), "Save output of the rpc calls made by benchmark: timeGetRandom")
		;
		// clang-format on

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		FunctionType functionType = FunctionType::noType;
		BenchmarkType benchmarkType = BenchmarkType::noType;
		std::string rawBenchmarkType = "";
		TestType testType = TestType::noType;
		std::string rawTestType = "";
		std::vector<std::string> benchmarkParameters = {};
		std::vector<std::string> testParameters = {};
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
			if (test.size() > 0) {
				rawTestType = test[0];
				testParameters = test;
				testParameters.erase(testParameters.begin());
			}
			testType = stringToTestType(rawTestType);
		} else if (help) {
			std::cout << desc << "\n";
		}
		config = Config(functionType, testType, testParameters, benchmarkType,
						benchmarkParameters, repetitions, outputFile, outputDir,
						help, showStatus, save, rawTestType, rawBenchmarkType);
		// config.printConfig(true);
		return;
	} catch (const po::error& ex) {
		std::cerr << ex.what() << '\n';
	}
	return;
}