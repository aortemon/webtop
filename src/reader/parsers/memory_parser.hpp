#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace webtop::reader {

struct MemoryData {
  uint64_t mem_total_kb;
  uint64_t mem_free_kb;
  uint64_t mem_available_kb;
  uint64_t mem_cached_kb;
  uint64_t mem_buffers_kb;
  uint64_t swap_total_kb;
  uint64_t swap_free_kb;
  uint64_t swap_cached_kb;

  [[nodiscard]] uint64_t MemTotalMb() const { return mem_total_kb / 1024; }
  [[nodiscard]] uint64_t MemFreeMb() const { return mem_free_kb / 1024; }
  [[nodiscard]] uint64_t MemAvailableMb() const {
    return mem_available_kb / 1024;
  }
  [[nodiscard]] uint64_t MemUsedKb() const {
    return mem_total_kb - mem_available_kb;
  }
  [[nodiscard]] uint64_t MemUsedMb() const { return MemUsedKb() / 1024; }
  [[nodiscard]] double MemUsedPercent() const {
    return (mem_total_kb > 0) ? (100.0 * static_cast<double>(MemUsedKb()) /
                                 static_cast<double>(mem_total_kb))
                              : 0.0;
  }

  [[nodiscard]] uint64_t SwapUsedKb() const {
    return swap_total_kb - swap_free_kb;
  }
  [[nodiscard]] uint64_t SwapUsedMb() const { return SwapUsedKb() / 1024; }
  [[nodiscard]] uint64_t SwapTotalMb() const { return swap_total_kb / 1024; }
  [[nodiscard]] double SwapUsedPercent() const {
    return (swap_total_kb > 0) ? (100.0 * static_cast<double>(SwapUsedKb()) /
                                  static_cast<double>(swap_total_kb))
                               : 0.0;
  }
};

class MemoryParser {
  static std::map<std::string, uint64_t> ParseMemInfo();

public:
  static MemoryData Read();
};

} // namespace webtop::reader
