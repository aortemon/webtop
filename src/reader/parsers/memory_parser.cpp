#include "reader/parsers/memory_parser.hpp"

#include <cstdint>
#include <map>
#include <sstream>
#include <string>

#include "reader/file_reader.hpp"

namespace webtop::reader {

std::map<std::string, uint64_t> MemoryParser::ParseMemInfo() {
  std::map<std::string, uint64_t> result;

  auto lines = FileReader::ReadLines("/proc/meminfo");
  for (const auto &line : lines) {
    if (line.empty()) {
      continue;
    }

    std::istringstream iss(line);
    std::string key;
    uint64_t value = 0;
    std::string unit;

    iss >> key >> value >> unit;

    if (!key.empty() && key.back() == ':') {
      key.pop_back();
    }

    result[key] = value;
  }

  return result;
}

MemoryData MemoryParser::Read() {
  auto meminfo = ParseMemInfo();

  MemoryData data = {};

  data.mem_total_kb = meminfo["MemTotal"];
  data.mem_free_kb = meminfo["MemFree"];
  data.mem_available_kb = meminfo["MemAvailable"];
  data.mem_cached_kb = meminfo["Cached"];
  data.mem_buffers_kb = meminfo["Buffers"];
  data.swap_total_kb = meminfo["SwapTotal"];
  data.swap_free_kb = meminfo["SwapFree"];
  data.swap_cached_kb = meminfo["SwapCached"];

  return data;
}

} // namespace webtop::reader
