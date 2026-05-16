#pragma once

#include "analizers/cpu_analizer.hpp"
#include "analizers/network_analizer.hpp"
#include "analizers/proc_analizer.hpp"
#include "common_snapshot.hpp"
#include <cstdint>

namespace webtop::aggregator {

class MetricsAggregator {
public:
  explicit MetricsAggregator();

  SystemSnapshot GetCurrentSnapshot();

  FullProcessSnapshot GetFullProcessSnapshot();

  void Reset();

private:
  CpuAnalizer cpu_analizer_;
  NetworkAnalizer network_analizer_;
  ProcAnalizer proc_analizer_;

  static uint64_t GetCurrentTimestamp();
};

} // namespace webtop::aggregator
