#include "aggregator.hpp"

#include <chrono>
#include <cstdint>

#include "aggregator/builders/disk_builder.hpp"
#include "aggregator/builders/loadavg_builder.hpp"
#include "aggregator/builders/memory_builder.hpp"
#include "aggregator/builders/proc_builder.hpp"
#include "aggregator/builders/uptime_builder.hpp"
#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/cpu_parser.hpp"
#include "reader/parsers/network_parser.hpp"

namespace webtop::aggregator {

MetricsAggregator::MetricsAggregator() = default;

uint64_t MetricsAggregator::GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
SystemSnapshot MetricsAggregator::GetCurrentSnapshot() {
  SystemSnapshot snapshot;

  auto current_cpu = reader::CpuParser::ReadAll();
  auto current_network = reader::NetworkParser::ReadAll();
  auto procs = ProcBuilder::Build();

  snapshot.timestamp = GetCurrentTimestamp();
  snapshot.total_procs_count = procs.total_count;
  snapshot.total_threads_count = procs.thread_count;

  snapshot.cpu = cpu_analizer_.Compute(current_cpu);
  snapshot.network = network_analizer_.Compute(current_network);
  snapshot.memory = MemoryBuilder::Build();
  snapshot.disks = DiskBuilder::Build();
  snapshot.loadavg = LoadAvgBuilder::Build();

  snapshot.uptime_formatted = UptimeBuilder::Build();

  return snapshot;
}

FullProcessSnapshot MetricsAggregator::GetFullProcessSnapshot() {
  auto snapshot = ProcBuilder::Build();
  auto current_cpu = reader::CpuParser::ReadAll();
  snapshot.memory = MemoryBuilder::Build();
  snapshot.cpu = cpu_analizer_.Compute(current_cpu);
  snapshot = proc_analizer_.Compute(snapshot);
  return snapshot;
}

void MetricsAggregator::Reset() {
  cpu_analizer_.Reset();
  network_analizer_.Reset();
}

} // namespace webtop::aggregator
