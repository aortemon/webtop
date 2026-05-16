#include <chrono>
#include <mutex>

#include "aggregator/common_snapshot.hpp"


namespace webtop::aggregator {

class ProcAnalizer {
  std::mutex mutex_;
  bool first_run_;
  FullProcessSnapshot prev_;
  std::chrono::time_point<std::chrono::steady_clock> prev_time_;

 public:
  ProcAnalizer();
  FullProcessSnapshot Compute(const FullProcessSnapshot &current);
  void Reset();
};
} // namespace webtop::aggregator
