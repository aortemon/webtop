#include "proc_builder.hpp"

#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdint>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/process_parser.hpp"
#include "reader/parsers/uptime_parser.hpp"

namespace webtop::aggregator {

namespace {
uint64_t GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
} // namespace

FullProcessSnapshot ProcBuilder::Build() {
  FullProcessSnapshot snapshot;
  snapshot.timestamp = GetCurrentTimestamp();
  snapshot.total_count = reader::ProcessParser::GetProcessCount();
  snapshot.thread_count = reader::ProcessParser::GetThreadCount();

  auto all_procs = reader::ProcessParser::ReadAll();

  snapshot.running_total = std::ranges::count_if(
      all_procs, [](const auto &x) { return x.state == 'R'; });
  snapshot.sleeping_total = std::ranges::count_if(
      all_procs, [](const auto &x) { return x.state == 'S'; });
  snapshot.stopped_total = std::ranges::count_if(
      all_procs, [](const auto &x) { return x.state == 'T'; });
  snapshot.zombie_total = std::ranges::count_if(
      all_procs, [](const auto &x) { return x.state == 'Z'; });

  auto system_uptime_ticks =
      static_cast<uint64_t>(reader::UptimeParser::Read().uptime_seconds *
                            static_cast<double>(sysconf(_SC_CLK_TCK)));

  for (const auto &proc : all_procs) {
    FullProcessInfo info;
    info.pid = proc.pid;
    info.ppid = proc.ppid;
    info.uid = proc.uid;
    info.user = proc.user;
    info.name = proc.name;
    info.state = proc.state;
    info.memory = proc.rss_pages * sysconf(_SC_PAGESIZE) / 1024 / 1024;
    info.cpu_percent = 0.0;
    info.vmem = proc.vmem_bytes / 1024 / 1024;
    info.threads = proc.num_threads;
    info.uptime_seconds = (system_uptime_ticks - proc.starttime_ticks) /
                          static_cast<uint64_t>(sysconf(_SC_CLK_TCK));
    info.total_ticks = proc.utime + proc.stime + proc.cutime + proc.cstime;
    snapshot.processes.push_back(info);
  }

  return snapshot;
}

} // namespace webtop::aggregator
