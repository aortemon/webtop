#include "aggregator/analizers/proc_analizer.hpp"

#include <unistd.h>

#include <algorithm>
#include <mutex>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/cpu_parser.hpp"

namespace webtop::aggregator {
ProcAnalizer::ProcAnalizer() : first_run_(true) {}

void ProcAnalizer::Reset() { first_run_ = true; }

FullProcessSnapshot ProcAnalizer::Compute(const FullProcessSnapshot &current) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto result = current;
  result.system_total = reader::CpuReader::ReadAll()[0].Total();
  if (first_run_) {
    first_run_ = false;
    prev_ = result;
    return result;
  }

  for (auto &proc : result.processes) {
    auto it = std::ranges::find_if(prev_.processes, [&proc](const auto &item) {
      return item.pid == proc.pid;
    });
    if (it != prev_.processes.end()) {
      proc.cpu_percent =
          100.0 * static_cast<double>(proc.total_ticks - it->total_ticks) /
          static_cast<double>(result.system_total - prev_.system_total) *
          static_cast<double>(sysconf(_SC_NPROCESSORS_CONF));
    }
  }
  prev_ = result;
  return result;
}
} // namespace webtop::aggregator
