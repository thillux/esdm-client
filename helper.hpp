#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <filesystem>

enum benchmarkType{
  pr,
  me,
  ts,
  noType
};


using ms = std::chrono::milliseconds;
using mis = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using Clock = std::chrono::high_resolution_clock;
using meOut = std::vector<std::pair<int64_t,int>>;