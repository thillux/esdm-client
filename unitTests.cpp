
#define CATCH_CONFIG_MAIN
#include "functions.hpp"
#include "writer.hpp"
#include <boost/program_options.hpp>
#include <catch2/catch.hpp>

// void makeConfig(int argc, char* argv[]){
Config makeConfig(std::string argumentString) {
	// extract 'int argc' and 'char* argv[]' from argumentString
	// we take the input string, split it and then insert the cStrings into
	// char* argv[]
	std::istringstream input;
	input.str(argumentString);
	std::vector<std::string> parameters;
	int argc = 0;
	for (std::string entry; std::getline(input, entry, ' ');) {
		parameters.push_back(entry);
		argc++;
	}
	char* argv[argc];
	for (int i = 1; i < argc; i++) {
		argv[i] = parameters[i].data();
	}

	Config config(FunctionType::noType, TestType::noType, {},
				  BenchmarkType::noType, {}, 0, "", "", false, false, false);
	createConfigFromArgs(argc, argv, config);
	// config.printConfig();
	return config;
}

TEST_CASE("ConfigBenchmarkValid::TimeGetRandom", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeGetRandom --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeGetRandom);
	REQUIRE(config.getBenchmarkParameters().empty());
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeGetRandom") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == true);
	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkValid::TimeGetRandomOneParameter",
		  "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeGetRandom 10";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeGetRandom);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({"10"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeGetRandom") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);
	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkValid::TimeGetRandomTwoParameters",
		  "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeGetRandom 10 16";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeGetRandom);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({"10", "16"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeGetRandom") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);
	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkInvalid::TimeGetRandomTooManyParameters",
		  "[config][benchmark]") {
	std::string testConfigStringTooManyParameters =
		"./esdm-client --b timeGetRandom 10 16 1";
	Config config = makeConfig(testConfigStringTooManyParameters);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeGetRandom);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({"10", "16", "1"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeGetRandom") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool validExecution = true;
	validExecution = callBenchmark(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigBenchmarkValid::MeasureEntropy", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b measureEntropy --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::measureEntropy);
	REQUIRE(config.getBenchmarkParameters().empty());
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("measureEntropy") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == true);

	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkValid::MeasureEntropyOneParameter",
		  "[config][benchmark]") {
	std::string testConfigStringOneParameters =
		"./esdm-client --b measureEntropy 5";
	Config config = makeConfig(testConfigStringOneParameters);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::measureEntropy);
	REQUIRE(config.getBenchmarkParameters()[0] == "5");
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("measureEntropy") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkValid::MeasureEntropyTwoParameters",
		  "[config][benchmark]") {
	std::string testConfigStringTwoParameters =
		"./esdm-client --b measureEntropy 10 100";
	Config config = makeConfig(testConfigStringTwoParameters);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::measureEntropy);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({"10", "100"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("measureEntropy") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkInvalid::MeasureEntropyTooManyParameters",
		  "[config][benchmark]") {
	std::string testConfigStringTooManyParameters =
		"./esdm-client --b measureEntropy 1 2 3";
	Config config = makeConfig(testConfigStringTooManyParameters);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::measureEntropy);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({"1", "2", "3"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("measureEntropy") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool validExecution = true;
	validExecution = callBenchmark(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigBenchmarkInvalid::TimeToSeedTooManyParameters",
		  "[config][benchmark]") {
	std::string testConfigStringTooManyParameters =
		"./esdm-client --b timeToSeed 1";
	Config config = makeConfig(testConfigStringTooManyParameters);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeToSeed);
	REQUIRE(config.getBenchmarkParameters() == std::vector<std::string>({"1"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeToSeed") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool validExecution = true;
	validExecution = callBenchmark(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigBenchmarkInvalid::TimeToSeed", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeToSeed --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeToSeed);
	REQUIRE(config.getBenchmarkParameters().empty());
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeToSeed") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == true);

	bool success = false;
	success = callBenchmark(config);
	REQUIRE(success);
}

TEST_CASE("ConfigBenchmarkInvalid", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::noType);
	REQUIRE(config.getBenchmarkParameters().empty());
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("") == 0);
	REQUIRE(config.getRepetitions() == 1);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == false);

	bool validExecution = true;
	validExecution = callBenchmark(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigTestValid::testSeed", "[config][test]") {
	std::string testConfigString = "./esdm-client --t testSeed";
	Config config = makeConfig(testConfigString);

	bool success = false;
	success = callTest(config);
	REQUIRE(success);
}

TEST_CASE("ConfigTestInvalidUnknownType", "[config][test]") {
	std::string testConfigString = "./esdm-client --t notARealTestFunction";
	Config config = makeConfig(testConfigString);

	bool configSetCorrect = config.getTestType() == TestType::unknownType;
	REQUIRE(configSetCorrect);
	bool validExecution = true;
	validExecution = callTest(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigTestInvalidNoType", "[config][test]") {
	std::string testConfigString = "./esdm-client --t ";
	Config config = makeConfig(testConfigString);

	bool configSetCorrect = config.getTestType() == TestType::noType;
	REQUIRE(configSetCorrect);
	bool validExecution = true;
	validExecution = callTest(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConfigTestsValid", "[config][test]") {
	std::string configStringTestRandBytes = "./esdm-client --t testRandBytes";
	Config configTestRandBytes = (makeConfig(configStringTestRandBytes));
	REQUIRE(callTest(configTestRandBytes));

	std::string configStringTestRandBytesFull =
		"./esdm-client --t testRandBytesFull";
	Config configTestRandBytesFull =
		(makeConfig(configStringTestRandBytesFull));
	REQUIRE(callTest(configTestRandBytesFull));

	std::string configStringTestRandBytesMin =
		"./esdm-client --t testRandBytesMin";
	Config configTestRandBytesMin = (makeConfig(configStringTestRandBytesMin));
	REQUIRE(callTest(configTestRandBytesMin));

	std::string configStringTestRandBytesPr =
		"./esdm-client --t testRandBytesPr";
	Config configTestRandBytesPr = (makeConfig(configStringTestRandBytesPr));
	REQUIRE(callTest(configTestRandBytesPr));

	std::string configStringTestWriteData = "./esdm-client --t testWriteData";
	Config configTestWriteData = (makeConfig(configStringTestWriteData));
	REQUIRE(callTest(configTestWriteData));

	std::string configStringTestStatus = "./esdm-client --t testStatus";
	Config configTestStatus = (makeConfig(configStringTestStatus));
	REQUIRE(callTest(configTestStatus));

	std::string configStringTestSeed = "./esdm-client --t testSeed";
	Config configTestSeed = (makeConfig(configStringTestSeed));
	REQUIRE(callTest(configTestSeed));

	std::string configStringTestEntCnt = "./esdm-client --t testEntCnt";
	Config configTestEntCnt = (makeConfig(configStringTestEntCnt));
	REQUIRE(callTest(configTestEntCnt));

	std::string configStringTestGetPoolsize =
		"./esdm-client --t testGetPoolsize";
	Config configTestGetPoolsize = (makeConfig(configStringTestGetPoolsize));
	REQUIRE(callTest(configTestGetPoolsize));

	std::string configStringTestGetWriteWakeupThresh =
		"./esdm-client --t testGetWriteWakeupThresh";
	Config configTestGetWriteWakeupThresh =
		(makeConfig(configStringTestGetWriteWakeupThresh));
	REQUIRE(callTest(configTestGetWriteWakeupThresh));

	std::string configStringTestGetMinReseedSecs =
		"./esdm-client --t testGetMinReseedSecs";
	Config configTestGetMinReseedSecs =
		(makeConfig(configStringTestGetMinReseedSecs));
	REQUIRE(callTest(configTestGetMinReseedSecs));
}

TEST_CASE("ConfigTestsValidWithParameter", "[config][test]") {
	std::string configStringTestRandBytes =
		"./esdm-client --t testRandBytes 64";
	Config configTestRandBytes = (makeConfig(configStringTestRandBytes));
	REQUIRE(callTest(configTestRandBytes));

	std::string configStringTestRandBytesFull =
		"./esdm-client --t testRandBytesFull 64";
	Config configTestRandBytesFull =
		(makeConfig(configStringTestRandBytesFull));
	REQUIRE(callTest(configTestRandBytesFull));

	std::string configStringTestRandBytesMin =
		"./esdm-client --t testRandBytesMin 64";
	Config configTestRandBytesMin = (makeConfig(configStringTestRandBytesMin));
	REQUIRE(callTest(configTestRandBytesMin));

	std::string configStringTestRandBytesPr =
		"./esdm-client --t testRandBytesPr 64";
	Config configTestRandBytesPr = (makeConfig(configStringTestRandBytesPr));
	REQUIRE(callTest(configTestRandBytesPr));

	std::string configStringTestWriteData =
		"./esdm-client --t testWriteData abc";
	Config configTestWriteData = (makeConfig(configStringTestWriteData));
	REQUIRE(callTest(configTestWriteData));
}

TEST_CASE("ConfigTestsPrivValid", "[sudo][config][test]") {
	std::string configStringTestPrivAddEntropy =
		"./esdm-client --t testPrivAddEntropy";
	Config configTestPrivAddEntropy =
		(makeConfig(configStringTestPrivAddEntropy));
	REQUIRE(callTest(configTestPrivAddEntropy));

	std::string configStringTestPrivAddToEntCnt =
		"./esdm-client --t testPrivAddToEntCnt";
	Config configTestPrivAddToEntCnt =
		(makeConfig(configStringTestPrivAddToEntCnt));
	REQUIRE(callTest(configTestPrivAddToEntCnt));

	std::string configStringTestPrivClearPool =
		"./esdm-client --t testPrivClearPool";
	Config configTestPrivClearPool =
		(makeConfig(configStringTestPrivClearPool));
	REQUIRE(callTest(configTestPrivClearPool));

	std::string configStringTestPrivReseedCrng =
		"./esdm-client --t testPrivReseedCrng";
	Config configTestPrivReseedCrng =
		(makeConfig(configStringTestPrivReseedCrng));
	REQUIRE(callTest(configTestPrivReseedCrng));

	std::string configStringTestPrivSetWriteWakeupThresh =
		"./esdm-client --t testPrivSetWriteWakeupThresh";
	Config configTestPrivSetWriteWakeupThresh =
		(makeConfig(configStringTestPrivSetWriteWakeupThresh));
	REQUIRE(callTest(configTestPrivSetWriteWakeupThresh));

	std::string configStringTestPrivSetMinReseedSecs =
		"./esdm-client --t testPrivSetMinReseedSecs";
	Config configTestPrivSetMinReseedSecs =
		(makeConfig(configStringTestPrivSetMinReseedSecs));
	REQUIRE(callTest(configTestPrivSetMinReseedSecs));
}

TEST_CASE("ConfigTestsPrivValidWithParameter", "[sudo][config][test]") {
	std::string configStringTestPrivAddEntropy =
		"./esdm-client --t testPrivAddEntropy abc";
	Config configTestPrivAddEntropy =
		(makeConfig(configStringTestPrivAddEntropy));
	REQUIRE(callTest(configTestPrivAddEntropy));

	std::string configStringTestPrivAddToEntCnt =
		"./esdm-client --t testPrivAddToEntCnt 1";
	Config configTestPrivAddToEntCnt =
		(makeConfig(configStringTestPrivAddToEntCnt));
	REQUIRE(callTest(configTestPrivAddToEntCnt));

	std::string configStringTestPrivSetWriteWakeupThresh =
		"./esdm-client --t testPrivSetWriteWakeupThresh 1";
	Config configTestPrivSetWriteWakeupThresh =
		(makeConfig(configStringTestPrivSetWriteWakeupThresh));
	REQUIRE(callTest(configTestPrivSetWriteWakeupThresh));

	std::string configStringTestPrivSetMinReseedSecs =
		"./esdm-client --t testPrivSetMinReseedSecs 30";
	Config configTestPrivSetMinReseedSecs =
		(makeConfig(configStringTestPrivSetMinReseedSecs));
	REQUIRE(callTest(configTestPrivSetMinReseedSecs));
}

TEST_CASE("ConversionFunctions::TestType", "[config][misc]") {
	REQUIRE(TestType::testRandBytes ==
			stringToTestType(testTypeToString(TestType::testRandBytes)));
	REQUIRE(TestType::testRandBytesFull ==
			stringToTestType(testTypeToString(TestType::testRandBytesFull)));
	REQUIRE(TestType::testRandBytesMin ==
			stringToTestType(testTypeToString(TestType::testRandBytesMin)));
	REQUIRE(TestType::testRandBytesPr ==
			stringToTestType(testTypeToString(TestType::testRandBytesPr)));
	REQUIRE(TestType::testWriteData ==
			stringToTestType(testTypeToString(TestType::testWriteData)));
	REQUIRE(TestType::testStatus ==
			stringToTestType(testTypeToString(TestType::testStatus)));
	REQUIRE(TestType::testSeed ==
			stringToTestType(testTypeToString(TestType::testSeed)));
	REQUIRE(TestType::testEntCnt ==
			stringToTestType(testTypeToString(TestType::testEntCnt)));
	REQUIRE(TestType::testGetPoolsize ==
			stringToTestType(testTypeToString(TestType::testGetPoolsize)));
	REQUIRE(
		TestType::testGetWriteWakeupThresh ==
		stringToTestType(testTypeToString(TestType::testGetWriteWakeupThresh)));
	REQUIRE(TestType::testPrivSetWriteWakeupThresh ==
			stringToTestType(
				testTypeToString(TestType::testPrivSetWriteWakeupThresh)));
	REQUIRE(TestType::testGetMinReseedSecs ==
			stringToTestType(testTypeToString(TestType::testGetMinReseedSecs)));
	REQUIRE(
		TestType::testPrivSetMinReseedSecs ==
		stringToTestType(testTypeToString(TestType::testPrivSetMinReseedSecs)));
	REQUIRE(TestType::testPrivAddEntropy ==
			stringToTestType(testTypeToString(TestType::testPrivAddEntropy)));
	REQUIRE(TestType::testPrivAddToEntCnt ==
			stringToTestType(testTypeToString(TestType::testPrivAddToEntCnt)));
	REQUIRE(TestType::testPrivClearPool ==
			stringToTestType(testTypeToString(TestType::testPrivClearPool)));
	REQUIRE(TestType::testPrivReseedCrng ==
			stringToTestType(testTypeToString(TestType::testPrivReseedCrng)));
	REQUIRE(TestType::noType ==
			stringToTestType(testTypeToString(TestType::noType)));
	REQUIRE(TestType::unknownType ==
			stringToTestType(testTypeToString(TestType::unknownType)));
}

TEST_CASE("ConversionFunctions::BenchmarkType", "[config][misc]") {
	REQUIRE(BenchmarkType::timeGetRandom ==
			stringToBenchmarkType(
				benchmarkTypeToString(BenchmarkType::timeGetRandom)));
	REQUIRE(BenchmarkType::measureEntropy ==
			stringToBenchmarkType(
				benchmarkTypeToString(BenchmarkType::measureEntropy)));
	REQUIRE(BenchmarkType::timeToSeed ==
			stringToBenchmarkType(
				benchmarkTypeToString(BenchmarkType::timeToSeed)));
	REQUIRE(
		BenchmarkType::noType ==
		stringToBenchmarkType(benchmarkTypeToString(BenchmarkType::noType)));
	REQUIRE(BenchmarkType::unknownType ==
			stringToBenchmarkType(
				benchmarkTypeToString(BenchmarkType::unknownType)));
}

TEST_CASE("WriterFunctions", "[config][writer]") {
	std::string outputDir = "./res/";
	std::string outFileName = outputDir + "testOutput.json";
	makeOutputDir(outputDir);
	Json::Value root;
	root["test"] = "test entry for this field";
	Writer writer;
	bool written = writer.writeOutputFile(outFileName, root);
	REQUIRE(written);
	std::ifstream outputFile(outFileName, outputFile.in);
	Json::Value readJson;
	outputFile >> readJson;
	bool equal = root == readJson;
	REQUIRE(equal);
	root["otherField"] = "now we expect then not to be equal";
	equal = root == readJson;
	REQUIRE_FALSE(equal);
}

TEST_CASE("WriterFunctions::UseConfig", "[config][writer]") {
	// run benchmark
	unsigned int requests = 10;
	std::string requestsString = std::to_string(requests);
	std::string testConfigString = "./esdm-client --b timeGetRandom " +
								   requestsString + " 64 --save --r 4";
	Config config = makeConfig(testConfigString);
	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
	REQUIRE(config.getTestParameters().empty());
	REQUIRE(config.getBenchmarkType() == BenchmarkType::timeGetRandom);
	REQUIRE(config.getBenchmarkParameters() ==
			std::vector<std::string>({requestsString, "64"}));
	REQUIRE(config.getRawTestType().compare("") == 0);
	REQUIRE(config.getRawBenchmarkType().compare("timeGetRandom") == 0);
	REQUIRE(config.getRepetitions() == 4);
	REQUIRE(config.getOutputFileName() == "data.bench");
	REQUIRE(config.getOutputDirName() == "./res/");
	REQUIRE(config.getHelp() == false);
	REQUIRE(config.getStatus() == false);
	REQUIRE(config.getSave() == true);
	bool benchmarkSuccessful = callBenchmark(config);
	REQUIRE(benchmarkSuccessful);
	// read the output file into Json::Value and compare the number of entries
	// in the data field
	for (int i = 0; i < config.getRepetitions(); i++) {
		std::string outFile = createOutFileName(config, i) + ".json";
		std::ifstream outputFile(outFile, outputFile.in);
		Json::Value writtenFile;
		outputFile >> writtenFile;
		bool isArray = writtenFile["data"]["returnedValues"].isArray();
		REQUIRE(isArray);
		bool rightSize =
			writtenFile["data"]["returnedValues"].size() == requests;
		REQUIRE(rightSize);
	}
}
