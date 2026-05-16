#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <string>
#include <thread>

#include "aggregator/aggregator.hpp"
#include "logger/logger.hpp"
#include "reader/parsers/dotenv_parser.hpp"
#include "server/http_server.hpp"

struct ServerConfig {
  int port;
  bool colored_output;
  bool debug_printing;
  std::string static_dir;
  int common_stats_send_rate;
  int procs_stats_send_rate;
};

namespace {

std::atomic<bool> running(true);

void SignalHandler(int /*signal*/) {
  webtop::logger::Logger::Instance().Info("Shutting down...");
  running = false;
}

ServerConfig ReadServerConfig() {
  std::filesystem::path project_root = PROJECT_ROOT;
  ServerConfig config = {.port = 8000,
                         .colored_output = true,
                         .debug_printing = true,
                         .static_dir = project_root / "static",
                         .common_stats_send_rate = 2,
                         .procs_stats_send_rate = 2};
  auto dotenv = webtop::reader::DotEnvParser::ReadAll(project_root / ".env");
  for (const auto &[key, value] : dotenv) {
    if (key == "LISTEN_PORT") {
      config.port = stoi(value);
    } else if (key == "ENABLE_DEBUG_INFO") {
      config.debug_printing = value == "1";
    } else if (key == "ENABLE_COLORED_OUTPUT") {
      config.colored_output = value == "1";
    } else if (key == "STATIC_DIR") {
      config.static_dir = project_root / value;
    } else if (key == "COMMON_STATS_UPDATE_RATE") {
      config.common_stats_send_rate = std::stoi(value);
    } else if (key == "PROCS_STATS_UPDATE_RATE") {
      config.procs_stats_send_rate = std::stoi(value);
    }
  }
  return config;
}

} // namespace
int main() {
  auto config = ReadServerConfig();
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  webtop::logger::Logger::Instance().SetDebugPrinting(config.debug_printing);
  webtop::logger::Logger::Instance().SetColoredOutput(config.colored_output);

  webtop::aggregator::MetricsAggregator aggregator;
  webtop::server::HttpServer server(aggregator, config.port, config.static_dir,
                                    config.common_stats_send_rate,
                                    config.procs_stats_send_rate);

  if (!server.Start()) {
    return -1;
  };
  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  server.Stop();
  return 0;
}