#include "uptime_parser.hpp"
#include <string>

#include "reader/file_reader.hpp"

namespace webtop::reader {

UptimeStats UptimeParser::Read() {
  UptimeStats data = {.uptime_seconds = 0.0, .idle_seconds = 0.0};
  std::string content = FileReader::ReadFile("/proc/uptime");

  if (content.empty()) {
    return data;
  }

  auto tokens = FileReader::Split(content, ' ');
  if (!tokens.empty()) {
    data.uptime_seconds = std::stod(tokens[0]);
  }
  if (tokens.size() >= 2) {
    data.idle_seconds = std::stod(tokens[1]);
  }

  return data;
}

} // namespace webtop::reader
