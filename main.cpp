#include <array>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <esdm/esdm_rpc_client.h>
#include <sys/types.h>

#include <helper.hpp>

#ifndef OUTPUTNAME
#define OUTPUTNAME "esdm-client.benchmark"
#endif


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

void getRandomNumbers(std::vector<std::pair<size_t,std::string>> &v, int requestSize = 32){
  size_t ret = 0;
  std::vector<uint8_t> randBytes;
  randBytes.resize(requestSize);
  esdm_invoke(esdm_rpcc_get_random_bytes_pr(randBytes.data(), randBytes.size()));
  assert(ret > 0);
  std::string retStr;
  retStr.append("0x");
  for (size_t i = 0; i < randBytes.size(); ++i) {
    retStr.append(str(boost::format("%02x") % static_cast<int>(randBytes[i])));
  }
  v.push_back(std::make_pair(ret, retStr));
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

// void getSeed(std::vector<std::pair<size_t,std::string>> &v){
// }

//todo: save the output of the getRandomNumbers call and log to file? -> use if(v != nullptr)
int64_t benchmarkPr(std::vector<std::pair<size_t,std::string>>* v = NULL, size_t req = 1, int reqSize = 32){
  esdm_rpcc_init_unpriv_service(NULL);
  size_t requests = req;
  std::chrono::time_point<std::chrono::system_clock> start = Clock::now();
  for (size_t i = 0; i < requests; i++)
  {
    if(v != NULL){
      std::cout << "test2\n";
      getRandomNumbers(*v, reqSize);
    }
    else
      getRandomNumbers(reqSize);
  }
  std::chrono::time_point<std::chrono::system_clock> end = Clock::now();
  
  auto duration = end - start;
  std::chrono::nanoseconds m = std::chrono::duration_cast<ns>(duration);

  esdm_rpcc_fini_unpriv_service();
  return m.count();
}

//todo
void benchmarkMeassureEntropy(meOut &v, long time = -1, long deltaMs = -1){
  //todo:
  //-do a call OR wait? -> maybe call external programm get_raw_entropy with system()
  size_t ret = 0;
  esdm_rpcc_init_unpriv_service(NULL);
  char buf[2048];
  assert(ret == 0);
  bool fullySeeded = false;
  bool firstLoop = true;
  bool useDelta = deltaMs != -1 ? true : false;
  ns maxDur(0);
  if(time > 0)
    maxDur = std::chrono::duration_cast<ns>(ms(time));
  ns dur = ns(0);
  std::chrono::time_point<std::chrono::system_clock, ns> start = Clock::now();
  std::chrono::time_point<std::chrono::system_clock, ns> now = Clock::now();
  ns delta;
  if(useDelta)
    delta = std::chrono::duration_cast<ns>(mis(deltaMs));
  std::chrono::time_point<std::chrono::system_clock, ns> d1 = Clock::now();
  
  //save start date as the first tuple in our retVec: (startTime, -1)
  int64_t startNano = start.time_since_epoch().count();
  v.push_back(std::make_pair(startNano, -1));

  //if time is set to the default -1 then dur == maxDur
  //-> the loop will repeat till the esdm status says it is fully seeded
  while(!fullySeeded || (dur < maxDur)){
    d1 = Clock::now();
    if(std::chrono::duration_cast<ns>(d1 - now) < delta && !firstLoop && useDelta){
      continue;
    }
    now = Clock::now();

    /*
      todo: maybe call a function here to manipulate the entropy level, if possible. (like reset, decrease the entropy)
      maybe get_raw_entropy?
    */

    // unsigned int entCnt;
    // esdm_invoke(esdm_rpcc_rnd_get_ent_cnt(&entCnt));
    // std::cout << "entCnt:" << entCnt << "\n";

    esdm_invoke(esdm_rpcc_status(buf, sizeof(buf)));
    dur = std::chrono::duration_cast<ns>(now - start);
    //try to find if esdm is fully seeded
    char searchFullySeeded[] = "ESDM fully seeded: true";
    fullySeeded = strstr(buf, searchFullySeeded) != NULL ? true : false;

    //get the current entropy level through string parsing
    char searchEntLvL[] = "entropy level: ";
    char* bp = strstr(buf, searchEntLvL);
    char* numC = std::strtok(bp, "\n") + strlen(searchEntLvL);
    int num = atoi(numC);
    int64_t nowNano = now.time_since_epoch().count();

    // std::cout << "nowNano:" << nowNano << "\n";

    // std::cout << "status ent level:" << num << "\n";

    std::pair<int64_t,int> entry = std::make_pair(nowNano, num);
    v.push_back(entry);
    std::memset(buf, 0, sizeof(buf));
    firstLoop = false;
  }

  esdm_rpcc_fini_unpriv_service();
}

int64_t timeTillSeeded(){
  std::chrono::time_point<std::chrono::system_clock, ns> start = Clock::now();
  std::chrono::time_point<std::chrono::system_clock, ns> end = start;  
  size_t ret = 0;
  esdm_rpcc_init_unpriv_service(NULL);
  char buf[2048];
  assert(ret == 0);
  bool fullySeeded = false;
  bool firstLoop = true;

  std::array<uint8_t,64> rbuf;
  esdm_invoke(esdm_rpcc_get_random_bytes_pr(rbuf.data(), rbuf.size()));
  while (!fullySeeded){
    esdm_invoke(esdm_rpcc_status(buf, sizeof(buf)));
    char searchFullySeeded[] = "ESDM fully seeded: true";
    fullySeeded = strstr(buf, searchFullySeeded) != NULL ? true : false;
    if(fullySeeded &&firstLoop){
      firstLoop=false;
      break;
    }
    std::memset(buf, 0, sizeof(buf));
    if(fullySeeded)
      end = Clock::now();
  }
  
  ns dur = end - start;

  esdm_rpcc_fini_unpriv_service();
  return dur.count();
}

int start_pr(const std::vector<std::string> &v, std::string outname, bool saveRandomOutput = false){
  //execute 'pr' benchmark
  int64_t outPr = 0;
  if(v.size() > 3){
    std::cout << "to many arguments for the pr benchmark\n. Expected at most 3 Arguments:\n"
              << "(0) type: 'pr'\n"
              << "(1) requests\n"
              << "(2) requestSize\n";
    return -1;
  }

  std::vector<std::pair<size_t,std::string>> retVec;
  std::vector<std::pair<size_t,std::string>>* retVecP = saveRandomOutput ? &retVec : NULL;
  if(v.size() == 3){
    outPr = benchmarkPr(retVecP, std::stoi(v[1]), std::stoi(v[2]));
  }else if(v.size() == 2){
    std::cout << "test\n";
    outPr = benchmarkPr(retVecP, std::stoi(v[1]));
  }else{
    outPr = benchmarkPr(retVecP);
  }
  //save result
  FILE* f = fopen(outname.c_str(), "w");
  if(!f){
    std::cout << "Could not open output file.\n";
    return -1;
  }
  fprintf(f, "benchmark pr called with: requests:%d requestSize:%d\n", std::stoi(v[1]), std::stoi(v[2]));
  fprintf(f, "duration:\t%lu\n", outPr);
  for(auto i : retVec){
    fprintf(f, "%lu\t%s\n", i.first, i.second.c_str());
  }
  fclose(f);
  return 0;
}

int start_me(const std::vector<std::string> &v, std::string outname){
  //execute 'me' benchmark
  meOut outMe;
  if(v.size() > 3){
    std::cout << "to many arguments for the me benchmark\n. Expected at most 3 Arguments:\n"
        << "(0) type: 'me'\n"
        << "(1) duration: check the entropy level for 'duration' milliseconds\n"
        << "(2) delta: if set only check after 'delta' microseconds have passed\n";
    return -1;
  }else if(v.size() == 3){
    benchmarkMeassureEntropy(outMe, std::stoi(v[1]), std::stoi(v[2]));
  }else if(v.size() == 2){
    benchmarkMeassureEntropy(outMe, std::stoi(v[1]));
  }else if(v.size() == 1){
    benchmarkMeassureEntropy(outMe);
  }
  //save result
  FILE* f = fopen(outname.c_str(), "w");
  if(!f){
    std::cout << "Could not open output file.\n";
    return -1;
  }
  for(auto i : outMe){
    fprintf(f, "%lu\t%d\n", i.first, i.second);
  }
  fclose(f);
  return 0;
}

int start_ts(const std::vector<std::string> &v, std::string outname){
  int64_t outTs;
  //execute 'ts' benchmark
  if(v.size() > 1){
    std::cout << "to many arguments for the me benchmark\n. Expected at most 1 Argument:\n"
        << "(0) type: 'ts'\n";
  }else if(v.size() == 1){
    outTs = timeTillSeeded();
  }
  //save result
  FILE* f = fopen(outname.c_str(), "w");
  if(!f){
    std::cout << "Could not open output file.\n";
    return -1;
  }
  fprintf(f, "%lu\n", outTs);
  fclose(f);
  return 0;
}

using namespace boost::program_options;
int call_benchmark(const std::vector<std::string> &v, const boost::program_options::variables_map &vm){
  std::cout << "vector.size():" << v.size() << "\n";

  bool save = false;
  if(vm.count("save"))
    save = vm["save"].as<bool>();

  if(v.size() == 0){
    std::cout << "please specify a benchmark to run:\n"
              << "'pr(requests,requestSize)':\tCall esdm_rpcc_get_random_bytes_pr\n"
              << "\t requests:\tNumber of times the function is called\n"
              << "\t requestSize:\tAmount of requested random bytes\n"
              << "'me(time,deltaMs)':\t Read the entropy status of the esdm\n"
              << "\t time:\tFor how long the entropy status is meassured (default: until fully seeded)\n"
              << "\t deltaMs:\tOnly read the entropy status after 'deltaMs' milliseconds (default: 0) have passed since the last time\n"
              << "'ts()':\tMeassure the time till esdm is fully seeded\n";
    return -1;
  }
  
  int repetitions = -1;
  if(vm.count("r")){
    repetitions = vm["r"].as<int>();
  }

  benchmarkType t;
  std::string tStr = v[0];
  if(tStr == "pr")
    t = pr;
  else if(tStr == "me")
    t = me;
  else if(tStr == "ts")
    t = ts;
  else
    t = noType;

  std::string outdir = vm["od"].as<std::string>();
  bool mkdir;
  mkdir = std::filesystem::create_directory(outdir);
  if(mkdir){
    std::cout << "Create output directory:" << outdir << "\n";
  }
  for (int i = 0; i < repetitions; i++)
  {
    std::string iStr = std::to_string(i);
    std::string outname = outdir + tStr + "." + iStr + "." + vm["of"].as<std::string>(); 
    std::cout << outname << "\n"; 
    switch (t)
    {
    case pr:
      start_pr(v, outname, save);
      break;

    case me:
      start_me(v, outname);
      break;

    case ts:
      start_ts(v, outname);
      break;

    default:
      std::cout << "Unknown benchmark name:" << tStr << "\n";
      break;
    }
  }

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
  // }else if(testName == "getSeed"){ //todo: just a test remove this block later
  //   std::vector<std::pair<size_t,std::string>> v;
  //   getSeed(v);
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
      ("t,test",value<std::string>(),"call test functions")
      ("of", value<std::string>()->default_value("data.bench"), "Suffix of output file. Name of output: 'benchmarkType'.'repetition'.'suffix'")
      ("od", value<std::string>()->default_value("./res/"), "Result directory")
      ("save", value<bool>()->default_value(false), "Do save returned values from esdm_rpcc functions called in benchmarks: pr");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    // size_t rep = vm["r"].as<size_t>();

    if (vm.count("help"))
      std::cout << desc << '\n';
    else if (vm.count("b"))
      call_benchmark(vm["b"].as<std::vector<std::string>>(), vm);
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
