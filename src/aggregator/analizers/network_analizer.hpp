#pragma once

#include <chrono>
#include <vector>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/network_parser.hpp"


namespace webtop::aggregator {

class NetworkAnalizer {
  std::chrono::time_point<std::chrono::steady_clock> prev_time_;
  NetworkSnapshot prev_stats_{};
  bool first_run_;

 public:
  NetworkAnalizer();

  NetworkSnapshot Compute(const std::vector<reader::NetworkInterfaceStats> &current);

  void Reset();
};

} // namespace webtop::aggregator
