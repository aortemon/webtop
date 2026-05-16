#include "metrics_handler.hpp"


#include <atomic>
#include <nlohmann/json.hpp>
#include <string>

#include "aggregator/aggregator.hpp"
#include "nlohmann/json_fwd.hpp"
#include "server/handlers/sse_handler.hpp"

using Json = nlohmann::json;


namespace webtop::server {

MetricsHandler::MetricsHandler(aggregator::MetricsAggregator &aggregator,
                               std::atomic<bool> &running, int update_interval)
    : SseHandler(running, update_interval),
      aggregator_(aggregator) {}

MetricsHandler::~MetricsHandler() = default;

std::string MetricsHandler::GetData() {
  auto snapshot = aggregator_.GetCurrentSnapshot();

  Json j;
  j["timestamp"] = snapshot.timestamp;
  j["uptime_formatted"] = snapshot.uptime_formatted;

  j["cpu"]["usage_percent"] = snapshot.cpu.usage_percent;
  j["cpu"]["user_percent"] = snapshot.cpu.user_percent;
  j["cpu"]["system_percent"] = snapshot.cpu.system_percent;
  j["cpu"]["idle_percent"] = snapshot.cpu.idle_percent;
  j["cpu"]["iowait_percent"] = snapshot.cpu.iowait_percent;
  j["cpu"]["cores_percent"] = snapshot.cpu.cores_percent;

  j["memory"]["total_mb"] = snapshot.memory.total;
  j["memory"]["used_mb"] = snapshot.memory.used;
  j["memory"]["available_mb"] = snapshot.memory.available;
  j["memory"]["used_percent"] = snapshot.memory.used_percent;
  j["memory"]["swap_total_mb"] = snapshot.memory.swap_total;
  j["memory"]["swap_used_mb"] = snapshot.memory.swap_used;
  j["memory"]["swap_used_percent"] = snapshot.memory.swap_used_percent;

  j["loadavg"]["load_1min"] = snapshot.loadavg.load_1min;
  j["loadavg"]["load_5min"] = snapshot.loadavg.load_5min;
  j["loadavg"]["load_15min"] = snapshot.loadavg.load_15min;

  j["network"]["rx_mbps"] = snapshot.network.rx_mbps;
  j["network"]["tx_mbps"] = snapshot.network.tx_mbps;
  j["network"]["rx_bytes_total"] = snapshot.network.rx_bytes_total;
  j["network"]["tx_bytes_total"] = snapshot.network.tx_bytes_total;

  for (const auto &disk : snapshot.disks) {
    Json d;
    d["device"] = disk.device;
    d["mount_point"] = disk.mount_point;
    d["total_mb"] = disk.total_mb;
    d["used_mb"] = disk.used_mb;
    d["free_mb"] = disk.free_mb;
    d["used_percent"] = disk.used_percent;
    j["disks"].push_back(d);
  }

  j["processes"]["total_count"] = snapshot.total_procs_count;
  j["processes"]["thread_count"] = snapshot.total_threads_count;

  return j.dump();
}

} // namespace webtop::server
