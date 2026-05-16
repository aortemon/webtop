#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace webtop::aggregator {

struct CpuSnapshot {
  double usage_percent;
  double user_percent;
  double system_percent;
  double idle_percent;
  double iowait_percent;
  std::vector<double> cores_percent;
};

struct MemorySnapshot {
  uint64_t total;
  uint64_t used;
  uint64_t available;
  double used_percent;
  uint64_t swap_total;
  uint64_t swap_used;
  double swap_used_percent;
  uint64_t buff_cache;
};

struct DiskSnapshot {
  std::string device;
  std::string mount_point;
  uint64_t total_mb;
  uint64_t used_mb;
  uint64_t free_mb;
  double used_percent;
};

struct NetworkSnapshot {
  double rx_mbps;
  double tx_mbps;
  uint64_t rx_bytes_total;
  uint64_t tx_bytes_total;
};

struct TopProcess {
  int32_t pid;
  std::string name;
  uint64_t memory;
  double cpu_percent;
};

struct ProcessSnapshot {
  int32_t total_count;
  int32_t thread_count;
  std::vector<TopProcess> top_cpu;
};

struct LoadAvgSnapshot {
  double load_1min;
  double load_5min;
  double load_15min;
};

struct ProcessesCounts {
  int32_t total_count;
  int32_t thread_count;
};

struct SystemSnapshot {
  uint64_t timestamp;
  CpuSnapshot cpu;
  MemorySnapshot memory;
  std::vector<DiskSnapshot> disks;
  NetworkSnapshot network;
  LoadAvgSnapshot loadavg;

  int32_t total_procs_count;
  int32_t total_threads_count;

  std::string uptime_formatted;
};

struct FullProcessInfo {
  int32_t pid;
  int32_t ppid;
  int32_t uid;
  std::string name;
  std::string user;
  char state;
  uint64_t memory;
  double cpu_percent;
  uint64_t vmem;
  int32_t threads;
  uint64_t uptime_seconds;
  uint64_t total_ticks;
};

struct FullProcessSnapshot {
  uint64_t timestamp;
  int32_t total_count;
  int32_t thread_count;
  uint64_t system_total;
  uint32_t running_total;
  uint32_t sleeping_total;
  uint32_t stopped_total;
  uint32_t zombie_total;

  std::vector<FullProcessInfo> processes;

  CpuSnapshot cpu;
  MemorySnapshot memory;
};

} // namespace webtop::aggregator
