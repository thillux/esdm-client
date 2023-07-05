#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <esdm/esdm_rpc_client.h>
#include <iostream>
#include <sys/types.h>
#include <chrono>
#include <vector>

#ifndef OUTPUTNAME
#define OUTPUTNAME "esdm-client.benchmark"
#endif

using Ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;

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
  std::cout << "Warning: this is cyclic entropy feeding for testing purposes and should never be done in production!" << std::endl;
  esdm_invoke(esdm_rpcc_rnd_add_entropy(randBytes.data(), randBytes.size(), randBytes.size() * 8));
  assert(ret == 0);

  esdm_rpcc_fini_priv_service();
}

void testStatus() {
   esdm_rpcc_init_unpriv_service(NULL);

  ssize_t ret = 0;

  std::array<char, 8192> buffer;
  esdm_invoke(
      esdm_rpcc_status(buffer.data(), buffer.size()));

  if(ret == 0) {
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
  esdm_invoke(
      esdm_rpcc_get_seed(reinterpret_cast<uint8_t*>(randBytes.data()), randBytes.size() * sizeof(uint64_t), 0));
  std::cout << "Ret: " << ret << std::endl;

  if(ret > 2 * sizeof(uint64_t)) {
    std::cout << "returned buffer size: " << randBytes[0] << std::endl;
    std::cout << "returned collected bits: " << randBytes[1] << std::endl;

    std::cout << "0x";
    for (size_t i = 2; i < randBytes[0]; ++i) {
      std::cout << boost::format("%02x") % static_cast<int>(reinterpret_cast<uint8_t*>(&randBytes[0])[i]);
    }
    std::cout << std::endl;
  }

  esdm_rpcc_fini_unpriv_service();
}

void getRandomNumbers(int requestSize = 32, bool coutOn = false){
  size_t ret = 0;
  std::vector<uint8_t> randBytes;
  randBytes.resize(requestSize);
  esdm_invoke(esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
  assert(ret > 0);
  
  //todo: remove print before actual using this in benchmarking!
  if(coutOn){
    std::cout << "retValue:" << ret << "\n";
    if(ret > 0){
      std::cout << "0x";
      for (size_t i = 0; i < randBytes.size(); ++i) {
        std::cout << boost::format("%02x") % static_cast<int>(randBytes[i]);
      }
      std::cout << "\n";
    }
  }
}

int64_t benchmark(size_t req = 1, int reqSize = 32){
  esdm_rpcc_init_unpriv_service(NULL);
  size_t requests = req;
  int requestsSize = reqSize;
  std::chrono::time_point<std::chrono::system_clock> start = Clock::now();
  for (size_t i = 0; i < requests; i++)
  {
    getRandomNumbers(requestsSize);
  }
  std::chrono::time_point<std::chrono::system_clock> end = Clock::now();
  
  auto duration = end - start;
  std::chrono::nanoseconds m = std::chrono::duration_cast<Ns>(duration);

  std::cout << "duration:" << m.count() << "\n";
  esdm_rpcc_fini_unpriv_service();
  return m.count();
}

//todo
void meassureEntropy(std::vector<std::pair<int, int>> &v, long time = -1){
  //todo:
  //implement time span with argument 'long time'
  //-do a call OR wait?
  //-write to vector: -> start datum?
  size_t ret = 0;
  esdm_rpcc_init_unpriv_service(NULL);
  char buf[2048];
  esdm_invoke(esdm_rpcc_status(buf, sizeof(buf)));
  assert(ret == 0);
  bool fullySeeded = false;
  std::chrono::time_point<std::chrono::system_clock, Ns> start = Clock::now();

  while(!fullySeeded){
  //try to find if esdm is fully seeded
  char searchFullySeeded[] = "ESDM fully seeded: true";
  fullySeeded = strstr(buf, searchFullySeeded) != NULL ? true : false;

  //get the current entropy level through string parsing
  char searchEntLvL[] = "entropy level: ";
  char* bp = strstr(buf, searchEntLvL);
  char* numC = std::strtok(bp, "\n") + strlen(searchEntLvL);
  int num = atoi(numC);
  std::chrono::time_point<std::chrono::system_clock, Ns> now = Clock::now();
  int64_t nowNano = now.time_since_epoch().count();

  std::cout << bp << "| num:(" << numC <<  ")|(int)num:" << num << "|>>>now(nanosec):" << nowNano << "---fullySeeded:" << fullySeeded << "\n";


  std::pair<int,int> entry = std::make_pair(nowNano, num);
  v.push_back(entry);
  }

  esdm_rpcc_fini_unpriv_service();
}

void timeTillSeeded(){

}

/*
v[0]: str     benchmark type
v[1]: size_t  requests
v[2]: int     requestsSize
*/
using namespace boost::program_options;
int call_benchmark(const std::vector<std::string> &v, const boost::program_options::variables_map &vm){
  std::cout << "vector.size():" << v.size() << "\n";
  int64_t dur = 0;
  std::vector<std::pair<int, int>> retVec;

/*
Benchmark types:
-pr (reqNum, reqSize): time the duration of consecutive calls of esdm_rpcc_get_random_bytes_pr
-me (time): meassure the entropy of esdm over time (till full seeded?)
*/
  if (v[0] == "pr")
  {
    if(v.size() > 3){
      std::cout << "to many arguments for the pr benchmark\n. Expected at most 3 Arguments:\n"
                << "(0) type: 'pr'\n"
                << "(1) requests\n"
                << "(2) requestSize\n";
      return -1;
    }
    if(v.size() == 3){
      dur = benchmark(std::stoi(v[1]), std::stoi(v[2]));
    }else if(v.size() == 2){
      dur = benchmark(std::stoi(v[1]));
    }else{
      dur = benchmark();
    }

    //print output to file
    const char* outname = vm["of"].as<std::string>().c_str();
    // char* outname = "test";
    std::cout << "outname:" << outname << "\n"; 
    FILE* f = fopen(outname, "w");
    if(!f){
      std::cout << "Could not open output file.\n";
      return -1;
    }

    fprintf(f, "duration:\t%lu", dur);
    fclose(f);
  }
  else if(v[0] == "me"){
    if(v.size() > 2){
      std::cout << "to many arguments for the pr benchmark\n. Expected at most 2 Arguments:\n"
          << "(0) type: 'me'\n"
          << "(1) duration\n";
      return -1;
    }
    if(v.size() == 2){
      meassureEntropy(retVec, std::stoi(v[1]));
    }else if(v.size() == 1){
      meassureEntropy(retVec);
    }
  }else if(v[0] == "ts"){ //todo
    if(v.size() == 1){
      timeTillSeeded();
    }
  }
  // print the options in std::vector... &v
  // std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>{
  //   std::cout, "\n"});

  return 0;
}

void call_rng(int requestSize = 32, bool coutOn = false){
  esdm_rpcc_init_unpriv_service(NULL);
  getRandomNumbers(requestSize, coutOn);
  esdm_rpcc_fini_unpriv_service();
}

void call_test(std::string testName){
  if(testName == "testUnprivRand"){
    testUnprivRand();
  }else if(testName == "testPrivRand"){
    testPrivRand();
  }else if(testName == "testStatus"){
    testStatus();
  }else if(testName == "testSeed"){
    testSeed();
  }else{
    std::cout << "unkown test with name:'" << testName << "' called\n";
  }
}

int main(int argc, char* argv[]) {
  try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("b,benchmark", value<std::vector<std::string>>()->multitoken()->zero_tokens()->composing(), "Benchmark")
      ("r,repetitions", value<int>()->default_value(1), "Repetitions of choosen Programm") //<--todo
      ("rng", value<int>()->default_value(32), "call esdm_rpcc_get_random_bytes_pr with specified request size")
      ("s,status","call esdm_get_status()")
      ("t,test",value<std::string>(),"call test functions")
      ("of", value<std::string>()->default_value("out.tool"), "Name of output file");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // size_t rep = vm["r"].as<size_t>();

    if (vm.count("help"))
      std::cout << desc << '\n';
    else if (vm.count("b"))
      call_benchmark(vm["b"].as<std::vector<std::string>>(), vm);
    else if (vm.count("s"))
      testStatus();
    else if (vm.count("t"))
      call_test(vm["t"].as<std::string>());
    else if (vm.count("rng"))
      call_rng(vm["rng"].as<int>(),true);
    
  }
  catch (const error &ex)
  {
    std::cerr << ex.what() << '\n';
  }

}
