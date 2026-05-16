#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace webtop::reader {

struct NetworkInterfaceStats {
  std::string name;

  uint64_t rx_bytes;
  uint64_t rx_packets;
  uint64_t rx_errors;
  uint64_t rx_dropped;
  uint64_t rx_fifo;
  uint64_t rx_frame;
  uint64_t rx_compressed;
  uint64_t rx_multicast;

  uint64_t tx_bytes;
  uint64_t tx_packets;
  uint64_t tx_errors;
  uint64_t tx_dropped;
  uint64_t tx_fifo;
  uint64_t tx_collisions;
  uint64_t tx_carrier;
  uint64_t tx_compressed;
};

class NetworkReader {
  static NetworkInterfaceStats ParseNetDevLine(const std::string &line);
  static bool IsInterfaceLine(const std::string &line);

 public:
  static std::vector<NetworkInterfaceStats> ReadAll();
  static bool IsInterfaceDestDefault(const std::string &iname);
};

} // namespace webtop::reader
