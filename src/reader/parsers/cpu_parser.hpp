#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace webtop::reader {

struct CpuData {
  uint64_t user;
  uint64_t nice;
  uint64_t system;
  uint64_t idle;
  uint64_t iowait;
  uint64_t irq;
  uint64_t softirq;
  uint64_t steal;

  [[nodiscard]] uint64_t Total() const {
    return user + nice + system + idle + iowait + irq + softirq + steal;
  }

  [[nodiscard]] uint64_t IdleTotal() const { return idle + iowait; }
};

class CpuParser {
  static CpuData ParseCpuLine(const std::string &line);
  static bool IsCpuLine(const std::string &line);

public:
  static std::vector<CpuData> ReadAll();
  static std::vector<CpuData> ReadCores();
};

} // namespace webtop::reader
