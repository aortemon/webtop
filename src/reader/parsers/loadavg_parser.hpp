#pragma once

#include <cstdint>


namespace webtop::reader {

struct LoadAvgStats {
  double load_1min;
  double load_5min;
  double load_15min;
  int32_t running_processes;
  int32_t total_processes;
  int32_t last_pid;
};

class LoadAvgReader {
 public:
  static LoadAvgStats Read();
};

} // namespace webtop::reader
