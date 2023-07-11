#include <boost/program_options.hpp>

#define CATCH_CONFIG_RUNNER
#include "functions.hpp"
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	// todorm: remove test output
	std::cout << "c:" << argc << "\n";
	for (int i = 0; i < argc; i++)
		std::cout << "i(" << i << "):" << argv[i] << " ";

	Config config(FunctionType::noType, TestType::noType, BenchmarkType::noType,
				  {}, 0, "", "", false, false, false);
	createConfigFromArgs(argc, argv, config);

	bool exit = true;
	if (config.getFunctionType() == FunctionType::benchmark) {
		exit = callBenchmark(config);
	} else if (config.getFunctionType() == FunctionType::test) {
		exit = callTest(config);
	} else if (config.getStatus()) {
		testStatus();
		exit = true;
	}
	if (exit)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}
