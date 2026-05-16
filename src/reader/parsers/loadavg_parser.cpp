#include "reader/parsers/loadavg_parser.hpp"

#include <charconv>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

#include "reader/file_reader.hpp"

namespace webtop::reader {

LoadAvgStats LoadAvgParser::Read() {
  LoadAvgStats data = {.load_1min = 0.0,
                       .load_5min = 0.0,
                       .load_15min = 0.0,
                       .running_processes = 0,
                       .total_processes = 0,
                       .last_pid = 0};

  std::string content = FileReader::ReadFile("/proc/loadavg");
  if (content.empty()) {
    return data;
  }

  std::istringstream iss(content);

  iss >> data.load_1min;
  iss >> data.load_5min;
  iss >> data.load_15min;

  std::string processes;
  iss >> processes;

  size_t slash_pos = processes.find('/');
  if (slash_pos != std::string::npos) {
    std::string_view running(processes.data(), slash_pos);
    std::string_view total(processes.data() + slash_pos + 1,
                           processes.size() - slash_pos - 1);
    auto [ptr1, ec1] =
        std::from_chars(running.data(), running.data() + running.size(),
                        data.running_processes);
    if (ec1 != std::errc()) {
      data.running_processes = 0;
    }

    auto [ptr2, ec2] = std::from_chars(
        total.data(), total.data() + total.size(), data.total_processes);
    if (ec2 != std::errc()) {
      data.total_processes = 0;
    }
  }

  iss >> data.last_pid;

  return data;
}

} // namespace webtop::reader
