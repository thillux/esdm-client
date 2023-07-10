#pragma once
#include <fstream>
#include <json/json.h>
#include <string>

enum class FunctionType {
	benchmark,
	test,
	noType,
};

std::string functionTypeToString(FunctionType functionType) {
	if (functionType == FunctionType::benchmark)
		return "benchmark";
	else if (functionType == FunctionType::test)
		return "test";
	else
		return "noType";
}

enum class TestType {
	testUnprivRand,
	testPrivRand,
	testStatus,
	testSeed,
	testJson,
	testEntCnt,
	noType,
};

TestType stringToTestType(std::string testString) {
	if (testString == "testUnprivRand")
		return TestType::testUnprivRand;
	else if (testString == "testPrivRand")
		return TestType::testPrivRand;
	else if (testString == "testStatus")
		return TestType::testStatus;
	else if (testString == "testSeed")
		return TestType::testSeed;
	else if (testString == "testJson")
		return TestType::testJson;
	else if (testString == "testEntCnt")
		return TestType::testEntCnt;
	else
		return TestType::noType;
}

std::string testTypeToString(TestType testType) {
	if (testType == TestType::testUnprivRand)
		return "testUnprivRand";
	else if (testType == TestType::testPrivRand)
		return "testPrivRand";
	else if (testType == TestType::testStatus)
		return "testStatus";
	else if (testType == TestType::testSeed)
		return "testSeed";
	else if (testType == TestType::testJson)
		return "testJson";
	else if (testType == TestType::testEntCnt)
		return "testEntCnt";
	else
		return "noType";
}

enum class BenchmarkType {
	timeGetRandom,
	measureEntropy,
	timeToSeed,
	noType,
	unkownType
};

BenchmarkType stringToBenchmarkType(std::string benchmarkString) {
	if (benchmarkString == "timeGetRandom")
		return BenchmarkType::timeGetRandom;
	else if (benchmarkString == "measureEntropy")
		return BenchmarkType::measureEntropy;
	else if (benchmarkString == "timeToSeed")
		return BenchmarkType::timeToSeed;
	else if (benchmarkString == "")
		return BenchmarkType::noType;
	else
		return BenchmarkType::unkownType;
}

std::string benchmarkTypeToString(BenchmarkType benchmarkType) {
	if (benchmarkType == BenchmarkType::timeGetRandom)
		return "timeGetRandom";
	else if (benchmarkType == BenchmarkType::measureEntropy)
		return "measureEntropy";
	else if (benchmarkType == BenchmarkType::timeToSeed)
		return "timeToSeed";
	else if (benchmarkType == BenchmarkType::noType)
		return "noType";
	else
		return "unkownType";
}

class Config {
	FunctionType functionType;
	TestType testType;
	BenchmarkType benchmarkType;
	std::vector<std::string> benchmarkParameters;
	int repetitions;
	std::string outputFileName;
	std::string outputDirName;
	bool help;	 // only print the help description. do nothing else
	bool status; // print the status of esdm, and continue with other operations
				 // (benchmark or test execution)
	bool save; // save the output of the benchmark
  public:
	Config(FunctionType functionType, TestType testType,
		   BenchmarkType benchmarkType, int repetitions,
		   std::string outputFileName, std::string outputDirName, bool help,
		   bool status, bool save)
		: functionType(functionType), testType(testType),
		  benchmarkType(benchmarkType), repetitions(repetitions),
		  outputFileName(outputFileName), outputDirName(outputDirName),
		  help(help), status(status), save(save){};
	void printConfig();
	void printBenchmarkParameters();
	FunctionType getFunctionType() { return functionType; };
	TestType getTestType() { return testType; };
	BenchmarkType getBenchmarkType() { return benchmarkType; };
	std::vector<std::string> getBenchmarkParameters() {
		return benchmarkParameters;
	};
	int getRepetitions() { return repetitions; };
	std::string getOutputFileName() { return outputFileName; };
	std::string getOutputDirName() { return outputDirName; };
	bool getHelp() { return help; };
	bool getStatus() { return status; };
	bool getSave() { return save; };
	void setBenchmarkParameters(const std::vector<std::string> parameters) {
		benchmarkParameters = parameters;
	};
};

void Config::printBenchmarkParameters() {
	for (auto i : benchmarkParameters) {
		std::cout << i << "\t";
	}
	std::cout << "\n";
}

void Config::printConfig() {
	std::cout << "Configuration:\n"
			  << "\tfunctiontype:\t" << functionTypeToString(this->functionType)
			  << "\n"
			  << "\ttestType:\t" << testTypeToString(this->testType) << "\n"
			  << "\tbenchmarkType:\t"
			  << benchmarkTypeToString(this->benchmarkType) << "\n"
			  << "\tbenchmarkParameters:\t";
	printBenchmarkParameters();
	std::cout << "\trepetitions:\t" << this->repetitions << "\n"
			  << "\toutputFileName:\t" << this->outputFileName << "\n"
			  << "\toutputDirName:\t" << this->outputDirName << "\n"
			  << "\thelp:\t" << this->help << "\n"
			  << "\tstatus:\t" << this->status << "\n"
			  << "\tsave:\t" << this->save << "\n";
};
// inline FunctionType Config::getFunctionType() { return functionType; }
// inline TestType Config::getTestType() { return testType; }
// inline BenchmarkType Config::getBenchmarkType() { return benchmarkType; }
// inline int Config::getRepetitions() { return repetitions; }
// inline std::string Config::getOutputFileName() { return outputFileName; }
// inline std::string Config::getOutputDirName() { return outputDirName; }

class Writer {
  private:
	Config* config;

  public:
	void writeOutputFile(const std::string& outFileNamePrefix,
						 const Json::Value jsonOutput);
	bool writeOutputFile(const std::string& outFileNamePrefix,
						 const Json::Value jsonOutput, Config config);
};

void Writer::writeOutputFile(const std::string& outFileNamePrefix,
							 const Json::Value jsonOutput) {
	std::string outFileName = outFileNamePrefix + ".json";
	std::ofstream outputJson(outFileName, outputJson.out);
	if (!outputJson.is_open()) {
		std::cout << "Could not open output json file.\n";
	}
	outputJson << jsonOutput << "\n";
};

bool Writer::writeOutputFile(const std::string& outFileNamePrefix,
							 const Json::Value jsonOutput, Config config) {
	std::string outFileName = outFileNamePrefix + ".json";
	std::ofstream outputJson(outFileName, outputJson.out);
	if (!outputJson.is_open()) {
		std::cout << "Could not open output json file.\n";
		return false;
	}
	Json::Value root;
	root["typeOutput"] = functionTypeToString(config.getFunctionType());
	if (config.getFunctionType() == FunctionType::benchmark) {
		root["benchmarkType"] =
			benchmarkTypeToString(config.getBenchmarkType());
	}
	root["data"] = jsonOutput;
	outputJson << root << "\n";

	return true;
};