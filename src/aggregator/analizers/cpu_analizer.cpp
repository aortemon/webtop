#include "cpu_analizer.hpp"

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/cpu_parser.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>


namespace webtop::aggregator {

CpuAnalizer::CpuAnalizer() : first_run_(true) {}

double CpuAnalizer::CalculateCpuPercent(const reader::CpuData &prev,
                                        const reader::CpuData &current) {
  uint64_t prev_total = prev.Total();
  uint64_t prev_idle = prev.IdleTotal();
  uint64_t curr_total = current.Total();
  uint64_t curr_idle = current.IdleTotal();

  uint64_t total_diff = curr_total - prev_total;
  uint64_t idle_diff = curr_idle - prev_idle;

  if (total_diff == 0) {
    return 0.0;
  }

  return 100.0 * static_cast<double>(total_diff - idle_diff) / static_cast<double>(total_diff);
}

CpuSnapshot CpuAnalizer::Compute(const std::vector<reader::CpuData> &current) {
  CpuSnapshot snapshot = {.usage_percent=0.0, .user_percent=0.0, .system_percent=0.0, .idle_percent=0.0, .iowait_percent=0.0, .cores_percent={}};

  if (first_run_ || prev_data_.empty() || current.empty()) {
    prev_data_ = current;
    first_run_ = false;
    return snapshot;
  }

  const auto &prev_total = prev_data_[0];
  const auto &curr_total = current[0];

  uint64_t prev_all = prev_total.Total();
  uint64_t curr_all = curr_total.Total();
  uint64_t prev_idle = prev_total.IdleTotal();
  uint64_t curr_idle = curr_total.IdleTotal();

  uint64_t diff_all = curr_all - prev_all;
  uint64_t diff_idle = curr_idle - prev_idle;
  uint64_t diff_user = curr_total.user - prev_total.user;
  uint64_t diff_system = curr_total.system - prev_total.system;
  uint64_t diff_iowait = curr_total.iowait - prev_total.iowait;

  if (diff_all > 0) {
    snapshot.usage_percent = 100.0 * static_cast<double>(diff_all - diff_idle) / static_cast<double>(diff_all);
    snapshot.user_percent = 100.0 * static_cast<double>(diff_user) / static_cast<double>(diff_all);
    snapshot.system_percent = 100.0 * static_cast<double>(diff_system) / static_cast<double>(diff_all);
    snapshot.idle_percent = 100.0 * static_cast<double>(diff_idle) / static_cast<double>(diff_all);
    snapshot.iowait_percent = 100.0 * static_cast<double>(diff_iowait) / static_cast<double>(diff_all);
  }

  size_t core_count = std::min(prev_data_.size(), current.size()) - 1;
  for (size_t i = 0; i < core_count; ++i) {
    double core_percent = CalculateCpuPercent(prev_data_[i + 1], current[i + 1]);
    snapshot.cores_percent.push_back(core_percent);
  }

  prev_data_ = current;

  return snapshot;
}

void CpuAnalizer::Reset() {
  first_run_ = true;
  prev_data_.clear();
}

} // namespace webtop::aggregator
