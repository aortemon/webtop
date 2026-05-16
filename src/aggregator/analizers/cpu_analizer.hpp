#pragma once

#include <vector>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/cpu_parser.hpp"

namespace webtop::aggregator {

class CpuAnalizer {
  std::vector<reader::CpuData> prev_data_;
  bool first_run_;

  static double CalculateCpuPercent(const reader::CpuData &prev,
                             const reader::CpuData &current);

public:
  CpuAnalizer();

  CpuSnapshot Compute(const std::vector<reader::CpuData> &current);
  void Reset();
};

} // namespace webtop::aggregator
