#include "process_handler.hpp"


#include <atomic>
#include <nlohmann/json.hpp>
#include <string>

#include "aggregator/aggregator.hpp"
#include "nlohmann/json_fwd.hpp"
#include "server/handlers/sse_handler.hpp"

using Json = nlohmann::json;


namespace webtop::server {

ProcessesHandler::ProcessesHandler(aggregator::MetricsAggregator &aggregator,
                                   std::atomic<bool> &running, int update_interval)
    : SseHandler(running, update_interval),
      aggregator_(aggregator) {}

ProcessesHandler::~ProcessesHandler() = default;

std::string ProcessesHandler::GetData() {
  auto snapshot = aggregator_.GetFullProcessSnapshot();

  Json j;
  j["timestamp"] = snapshot.timestamp;

  j["total_count"] = snapshot.total_count;
  j["thread_count"] = snapshot.thread_count;
  j["running_total"] = snapshot.running_total;
  j["sleeping_total"] = snapshot.sleeping_total;
  j["stopped_total"] = snapshot.stopped_total;
  j["zombie_total"] = snapshot.zombie_total;

  j["cpu"]["usage_percent"] = snapshot.cpu.usage_percent;
  j["cpu"]["user_percent"] = snapshot.cpu.user_percent;
  j["cpu"]["system_percent"] = snapshot.cpu.system_percent;
  j["cpu"]["idle_percent"] = snapshot.cpu.idle_percent;
  j["cpu"]["iowait_percent"] = snapshot.cpu.iowait_percent;

  j["memory"]["total_mb"] = snapshot.memory.total;
  j["memory"]["used_mb"] = snapshot.memory.used;
  j["memory"]["available_mb"] = snapshot.memory.available;
  j["memory"]["used_percent"] = snapshot.memory.used_percent;
  j["memory"]["swap_total_mb"] = snapshot.memory.swap_total;
  j["memory"]["swap_used_mb"] = snapshot.memory.swap_used;
  j["memory"]["swap_used_percent"] = snapshot.memory.swap_used_percent;

  for (const auto &proc : snapshot.processes) {
    Json p;
    p["pid"] = proc.pid;
    p["ppid"] = proc.ppid;
    p["name"] = proc.name;
    p["user"] = proc.user;
    p["state"] = std::string(1, proc.state);
    p["memory"] = proc.memory;
    p["vmem"] = proc.vmem;
    p["cpu_percent"] = proc.cpu_percent;
    p["threads"] = proc.threads;
    p["uptime_seconds"] = proc.uptime_seconds;
    j["processes"].push_back(p);
  }

  return j.dump();
}

} // namespace webtop::server
