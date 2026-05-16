#include "aggregator/analizers/network_analizer.hpp"

#include <chrono>
#include <vector>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/network_parser.hpp"

namespace webtop::aggregator {

NetworkAnalizer::NetworkAnalizer() : first_run_(true) {}

NetworkSnapshot NetworkAnalizer::Compute(
    const std::vector<reader::NetworkInterfaceStats> &current) {
  NetworkSnapshot snapshot = {
      .rx_mbps = 0.0, .tx_mbps = 0.0, .rx_bytes_total = 0, .tx_bytes_total = 0};

  for (const auto &iface : current) {
    if (reader::NetworkParser::IsInterfaceDestDefault(iface.name)) {
      snapshot.rx_bytes_total += iface.rx_bytes;
      snapshot.tx_bytes_total += iface.tx_bytes;
    }
  }

  if (first_run_) {
    prev_stats_ = snapshot;
    first_run_ = false;
    prev_time_ = std::chrono::steady_clock::now();
    return snapshot;
  }

  auto current_time = std::chrono::steady_clock::now();
  auto interval = current_time - prev_time_;
  auto interval_seconds = std::chrono::duration<double>(interval).count();

  snapshot.rx_mbps = static_cast<double>(snapshot.rx_bytes_total -
                                         prev_stats_.rx_bytes_total) *
                     8 / (interval_seconds * 1000000);
  snapshot.tx_mbps = static_cast<double>(snapshot.tx_bytes_total -
                                         prev_stats_.tx_bytes_total) *
                     8 / (interval_seconds * 1'000'000);

  prev_stats_ = snapshot;
  return snapshot;
}

void NetworkAnalizer::Reset() { first_run_ = true; }

} // namespace webtop::aggregator
