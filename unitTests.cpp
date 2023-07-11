
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

	Config config(FunctionType::noType, TestType::noType, BenchmarkType::noType,
				  {}, 0, "", "", false, false, false);
	createConfigFromArgs(argc, argv, config);
	// config.printConfig();
	return config;
}

TEST_CASE("ConfigBenchmarkValid::TimeGetRandom", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeGetRandom --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
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

TEST_CASE("ConfigBenchmarkValid::MeasureEntropy", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b measureEntropy --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
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

TEST_CASE("ConfigBenchmarkInvalid::TimeToSeed", "[config][benchmark]") {
	std::string testConfigString = "./esdm-client --b timeToSeed --save";
	Config config = makeConfig(testConfigString);

	REQUIRE(config.getFunctionType() == FunctionType::benchmark);
	REQUIRE(config.getTestType() == TestType::noType);
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

TEST_CASE("ConfigTestValid::testUnprivRand", "[config][test]") {
	std::string testConfigString = "./esdm-client --t testUnprivRand";
	Config config = makeConfig(testConfigString);

	bool success = false;
	success = callTest(config);
	REQUIRE(success);
}

TEST_CASE("ConfigTestValid::testPrivRand", "[sudo][test]") {
	std::string testConfigString = "./esdm-client --t testPrivRand";
	Config config = makeConfig(testConfigString);

	bool success = false;
	success = callTest(config);
	REQUIRE(success);
}

TEST_CASE("ConfigTestValid::testStatus", "[config][test]") {
	std::string testConfigString = "./esdm-client --t testStatus";
	Config config = makeConfig(testConfigString);

	bool success = false;
	success = callTest(config);
	REQUIRE(success);
}

TEST_CASE("ConfigTestValid::testSeed", "[config][test]") {
	std::string testConfigString = "./esdm-client --t testSeed";
	Config config = makeConfig(testConfigString);

	bool success = false;
	success = callTest(config);
	REQUIRE(success);
}

TEST_CASE("ConfigTestInvalid", "[config][test]") {
	std::string testConfigString = "./esdm-client --t notARealTestFunction";
	Config config = makeConfig(testConfigString);

	bool configSetCorrect = config.getTestType() == TestType::unknownType;
	REQUIRE(configSetCorrect);
	bool validExecution = true;
	validExecution = callTest(config);
	REQUIRE_FALSE(validExecution);
}

TEST_CASE("ConversionFunctions::TestType", "[config][misc]") {
	REQUIRE(TestType::testUnprivRand ==
			stringToTestType(testTypeToString(TestType::testUnprivRand)));
	REQUIRE(TestType::testPrivRand ==
			stringToTestType(testTypeToString(TestType::testPrivRand)));
	REQUIRE(TestType::testStatus ==
			stringToTestType(testTypeToString(TestType::testStatus)));
	REQUIRE(TestType::testSeed ==
			stringToTestType(testTypeToString(TestType::testSeed)));
	REQUIRE(TestType::testJson ==
			stringToTestType(testTypeToString(TestType::testJson)));
	REQUIRE(TestType::testEntCnt ==
			stringToTestType(testTypeToString(TestType::testEntCnt)));
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
