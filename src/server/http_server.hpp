#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#include "aggregator/aggregator.hpp"
#include "server/router.hpp"


namespace webtop::server {

class MetricsHandler;
class ProcessesHandler;
class StaticHandler;
class SseHandler;

class FDCloseGuard {
  int fd_;

 public:
  explicit FDCloseGuard(int fd) : fd_(fd) {}
  ~FDCloseGuard() {
    if (fd_ > 0) {
      close(fd_);
    }
  }
  FDCloseGuard(const FDCloseGuard &) = delete;
  FDCloseGuard &operator=(const FDCloseGuard &) = delete;
};

class HttpServer {
  int common_update_rate_;
  int procs_update_rate_;
  int port_;
  int server_fd_;
  std::string static_root_;
  std::atomic<bool> running_;
  std::thread accept_thread_;

  aggregator::MetricsAggregator &aggregator_;
  Router router_;

  std::unique_ptr<MetricsHandler> metrics_handler_;
  std::unique_ptr<ProcessesHandler> processes_handler_;
  std::unique_ptr<StaticHandler> static_handler_;
  std::unique_ptr<SseHandler> sse_handler_;

  void PrintStartMessage();

  void AcceptLoop();
  void HandleClient(int client_fd);
  void HandleClientAsync(int client_fd);
  void SetupRoutes();

 public:
  HttpServer(aggregator::MetricsAggregator &aggregator, int port, std::string static_dir,
             int common_update_rate, int procs_update_rate);
  ~HttpServer();

  bool Start();
  void Stop();
};

} // namespace webtop::server
