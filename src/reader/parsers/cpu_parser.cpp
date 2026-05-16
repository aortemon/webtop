#include "cpu_parser.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "reader/file_reader.hpp"

namespace webtop::reader {

CpuData CpuParser::ParseCpuLine(const std::string &line) {
  CpuData data = {};
  std::istringstream iss(line);
  std::string label;

  iss >> label;
  iss >> data.user >> data.nice >> data.system >> data.idle >> data.iowait >>
      data.irq >> data.softirq >> data.steal;

  return data;
}

bool CpuParser::IsCpuLine(const std::string &line) {
  if (line.length() < 3) {
    return false;
  }
  return line.substr(0, 3) == "cpu";
}

std::vector<CpuData> CpuParser::ReadAll() {
  std::vector<CpuData> result;
  auto lines = FileReader::ReadLines("/proc/stat");

  for (const auto &line : lines) {
    if (IsCpuLine(line)) {
      result.push_back(ParseCpuLine(line));
    } else {
      break;
    }
  }

  return result;
}

std::vector<CpuData> CpuParser::ReadCores() {
  auto all = ReadAll();
  if (all.size() <= 1) {
    return {};
  }
  return {all.begin() + 1, all.end()};
}

} // namespace webtop::reader
