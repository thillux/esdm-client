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
	testRandBytes,
	testRandBytesFull,
	testRandBytesMin,
	testRandBytesPr,
	testWriteData,
	testStatus,
	testSeed,
	testEntCnt,
	testGetPoolsize,
	testGetWriteWakeupThresh,
	testPrivSetWriteWakeupThresh,
	testGetMinReseedSecs,
	testPrivSetMinReseedSecs,
	testPrivAddEntropy,
	testPrivAddToEntCnt,
	testPrivClearPool,
	testPrivReseedCrng,
	noType,
	unknownType,
};

TestType stringToTestType(std::string testString) {
	if (testString == "testRandBytes")
		return TestType::testRandBytes;
	else if (testString == "testRandBytesFull")
		return TestType::testRandBytesFull;
	else if (testString == "testRandBytesMin")
		return TestType::testRandBytesMin;
	else if (testString == "testRandBytesPr")
		return TestType::testRandBytesPr;
	else if (testString == "testWriteData")
		return TestType::testWriteData;
	else if (testString == "testStatus")
		return TestType::testStatus;
	else if (testString == "testSeed")
		return TestType::testSeed;
	else if (testString == "testEntCnt")
		return TestType::testEntCnt;
	else if (testString == "testGetPoolsize")
		return TestType::testGetPoolsize;
	else if (testString == "testGetWriteWakeupThresh")
		return TestType::testGetWriteWakeupThresh;
	else if (testString == "testPrivSetWriteWakeupThresh")
		return TestType::testPrivSetWriteWakeupThresh;
	else if (testString == "testGetMinReseedSecs")
		return TestType::testGetMinReseedSecs;
	else if (testString == "testPrivSetMinReseedSecs")
		return TestType::testPrivSetMinReseedSecs;
	else if (testString == "testPrivAddEntropy")
		return TestType::testPrivAddEntropy;
	else if (testString == "testPrivAddToEntCnt")
		return TestType::testPrivAddToEntCnt;
	else if (testString == "testPrivClearPool")
		return TestType::testPrivClearPool;
	else if (testString == "testPrivReseedCrng")
		return TestType::testPrivReseedCrng;
	else if (testString == "" || testString == "noType")
		return TestType::noType;
	else
		return TestType::unknownType;
}

std::string testTypeToString(TestType testType) {
	if (testType == TestType::testRandBytes)
		return "testRandBytes";
	else if (testType == TestType::testRandBytesMin)
		return "testRandBytesMin";
	else if (testType == TestType::testRandBytesFull)
		return "testRandBytesFull";
	else if (testType == TestType::testRandBytesPr)
		return "testRandBytesPr";
	else if (testType == TestType::testWriteData)
		return "testWriteData";
	else if (testType == TestType::testStatus)
		return "testStatus";
	else if (testType == TestType::testSeed)
		return "testSeed";
	else if (testType == TestType::testEntCnt)
		return "testEntCnt";
	else if (testType == TestType::testGetPoolsize)
		return "testGetPoolsize";
	else if (testType == TestType::testGetWriteWakeupThresh)
		return "testGetWriteWakeupThresh";
	else if (testType == TestType::testPrivSetWriteWakeupThresh)
		return "testPrivSetWriteWakeupThresh";
	else if (testType == TestType::testGetMinReseedSecs)
		return "testGetMinReseedSecs";
	else if (testType == TestType::testPrivSetMinReseedSecs)
		return "testPrivSetMinReseedSecs";
	else if (testType == TestType::testPrivAddEntropy)
		return "testPrivAddEntropy";
	else if (testType == TestType::testPrivAddToEntCnt)
		return "testPrivAddToEntCnt";
	else if (testType == TestType::testPrivClearPool)
		return "testPrivClearPool";
	else if (testType == TestType::testPrivReseedCrng)
		return "testPrivReseedCrng";
	else if (testType == TestType::noType)
		return "noType";
	else
		return " unknownType";
}

enum class BenchmarkType {
	timeGetRandom,
	measureEntropy,
	timeToSeed,
	noType,
	unknownType
};

BenchmarkType stringToBenchmarkType(std::string benchmarkString) {
	if (benchmarkString == "timeGetRandom")
		return BenchmarkType::timeGetRandom;
	else if (benchmarkString == "measureEntropy")
		return BenchmarkType::measureEntropy;
	else if (benchmarkString == "timeToSeed")
		return BenchmarkType::timeToSeed;
	else if (benchmarkString == "" || benchmarkString == "noType")
		return BenchmarkType::noType;
	else
		return BenchmarkType::unknownType;
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
		return "unknownType";
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
	std::string rawTestType;
	std::string rawBenchmarkType;

  public:
	Config(FunctionType functionType, TestType testType,
		   BenchmarkType benchmarkType,
		   std::vector<std::string> benchmarkParameters, int repetitions,
		   std::string outputFileName, std::string outputDirName, bool help,
		   bool status, bool save, std::string rawTestType = "",
		   std::string rawBenchmarkType = "")
		: functionType(functionType), testType(testType),
		  benchmarkType(benchmarkType),
		  benchmarkParameters(benchmarkParameters), repetitions(repetitions),
		  outputFileName(outputFileName), outputDirName(outputDirName),
		  help(help), status(status), save(save), rawTestType(rawTestType),
		  rawBenchmarkType(rawBenchmarkType){};
	void printConfig(bool printAll = false);
	void printBenchmarkParameters();

	// getters
	FunctionType getFunctionType() { return functionType; };
	TestType getTestType() { return testType; };
	BenchmarkType getBenchmarkType() { return benchmarkType; };
	std::vector<std::string> getBenchmarkParameters() {
		return benchmarkParameters;
	};
	std::string getRawTestType() { return rawTestType; };
	std::string getRawBenchmarkType() { return rawBenchmarkType; };
	int getRepetitions() { return repetitions; };
	std::string getOutputFileName() { return outputFileName; };
	std::string getOutputDirName() { return outputDirName; };
	bool getHelp() { return help; };
	bool getStatus() { return status; };
	bool getSave() { return save; };

	// setters
	void setFunctionType(const FunctionType type) {
		this->functionType = type;
	};
	void setTestType(const TestType type) { this->testType = type; };
	void setBenchmarkType(const BenchmarkType type) {
		this->benchmarkType = type;
	};
	void setBenchmarkParameters(const std::vector<std::string> parameters) {
		this->benchmarkParameters = parameters;
	};
	void setRawTestType(std::string typeString) {
		this->rawTestType = typeString;
	};
	void setRawBenchmarkType(std::string typeString) {
		this->rawBenchmarkType = typeString;
	};
	void setRepetitions(const int repetitions) {
		this->repetitions = repetitions;
	};
	void setOutputFileName(const std::string fileName) {
		this->outputFileName = fileName;
	};
	void setOutputDirName(const std::string dirName) {
		this->outputDirName = dirName;
	};
	void setHelp(const bool help) { this->help = help; };
	void setStatus(const bool status) { this->status = status; };
	void setSave(const bool save) { this->save = save; };
};

void Config::printBenchmarkParameters() {
	for (auto i : benchmarkParameters) {
		std::cout << i << "\t";
	}
	std::cout << "\n";
}

void Config::printConfig(bool printAll) {
	std::cout << "Configuration:\n"
			  << "\tfunctiontype:\t" << functionTypeToString(this->functionType)
			  << "\n"
			  << "\ttestType:\t" << testTypeToString(this->testType) << "\n";
	if (printAll)
		std::cout << "\trawTestType:\t" << this->rawTestType << "\n";
	std::cout << "\tbenchmarkType:\t"
			  << benchmarkTypeToString(this->benchmarkType) << "\n";
	if (printAll)
		std::cout << "\trawBenchmarkType:\t" << this->rawBenchmarkType << "\n";
	std::cout << "\tbenchmarkParameters:\t";
	printBenchmarkParameters();
	std::cout << "\trepetitions:\t" << this->repetitions << "\n"
			  << "\toutputFileName:\t" << this->outputFileName << "\n"
			  << "\toutputDirName:\t" << this->outputDirName << "\n"
			  << "\thelp:\t" << this->help << "\n"
			  << "\tstatus:\t" << this->status << "\n"
			  << "\tsave:\t" << this->save << "\n";
};

class Writer {
  public:
	bool writeOutputFile(const std::string& outFileNamePrefix,
						 const Json::Value jsonOutput);
	bool writeOutputFile(const std::string& outFileNamePrefix,
						 const Json::Value jsonOutput, Config config);
};

bool Writer::writeOutputFile(const std::string& outFileName,
							 const Json::Value jsonOutput) {
	std::ofstream outputJson(outFileName, outputJson.out);
	if (!outputJson.is_open()) {
		std::cout << "Could not open output json file.\n";
		return false;
	}
	outputJson << jsonOutput << "\n";
	return true;
};

bool Writer::writeOutputFile(const std::string& outFileNamePrefix,
							 const Json::Value jsonOutput, Config config) {
	std::string outFileName = outFileNamePrefix + ".json";
	Json::Value root;
	root["typeOutput"] = functionTypeToString(config.getFunctionType());
	if (config.getFunctionType() == FunctionType::benchmark) {
		root["benchmarkType"] =
			benchmarkTypeToString(config.getBenchmarkType());
	}
	root["data"] = jsonOutput;
	writeOutputFile(outFileName, root);

	return true;
};