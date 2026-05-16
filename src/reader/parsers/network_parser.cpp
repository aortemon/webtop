#include "reader/parsers/network_parser.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "reader/file_reader.hpp"

namespace webtop::reader {
bool NetworkParser::IsInterfaceLine(const std::string &line) {
  if (line.find("Inter-|") != std::string::npos) {
    return false;
  }
  if (line.find(" face |") != std::string::npos) {
    return false;
  }
  if (line.empty()) {
    return false;
  }

  return line.find(':') != std::string::npos;
}

NetworkInterfaceStats NetworkParser::ParseNetDevLine(const std::string &line) {
  NetworkInterfaceStats stats{};

  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos) {
    return stats;
  }

  stats.name = line.substr(0, colon_pos);

  size_t start = stats.name.find_first_not_of(" \t");
  size_t end = stats.name.find_last_not_of(" \t");
  if (start != std::string::npos && end != std::string::npos) {
    stats.name = stats.name.substr(start, end - start + 1);
  }

  std::string numbers_str = line.substr(colon_pos + 1);
  std::istringstream iss(numbers_str);

  iss >> stats.rx_bytes;
  iss >> stats.rx_packets;
  iss >> stats.rx_errors;
  iss >> stats.rx_dropped;
  iss >> stats.rx_fifo;
  iss >> stats.rx_frame;
  iss >> stats.rx_compressed;
  iss >> stats.rx_multicast;
  iss >> stats.tx_bytes;
  iss >> stats.tx_packets;
  iss >> stats.tx_errors;
  iss >> stats.tx_dropped;
  iss >> stats.tx_fifo;
  iss >> stats.tx_collisions;
  iss >> stats.tx_carrier;
  iss >> stats.tx_compressed;

  return stats;
}

bool NetworkParser::IsInterfaceDestDefault(const std::string &iname) {
  auto lines = FileReader::ReadLines("/proc/net/route");
  return std::ranges::any_of(lines, [&iname](const std::string &line) {
    return line.starts_with(iname);
  });
}

std::vector<NetworkInterfaceStats> NetworkParser::ReadAll() {
  std::vector<NetworkInterfaceStats> interfaces;

  auto lines = FileReader::ReadLines("/proc/net/dev");
  for (const auto &line : lines) {
    if (IsInterfaceLine(line)) {
      interfaces.push_back(ParseNetDevLine(line));
    }
  }

  return interfaces;
}

} // namespace webtop::reader
