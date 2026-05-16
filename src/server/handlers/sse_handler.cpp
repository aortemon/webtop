#include "sse_handler.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#include "logger/logger.hpp"

namespace webtop::server {

SseHandler::SseHandler(std::atomic<bool> &running, int update_interval)
    : running_(running), update_interval_(update_interval) {}

SseHandler::~SseHandler() = default;

void SseHandler::Handle(int client_fd, const std::string &path) {
  logger::Logger::Instance().Success("200\tGET\t" + path + "\t" +
                                     std::to_string(client_fd));
  std::string response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/event-stream\r\n";
  response += "Cache-Control: no-cache\r\n";
  response += "Connection: keep-alive\r\n";
  response += "Access-Control-Allow-Origin: *\r\n\r\n";

  logger::Logger::Instance().Debug(
      "Sending initial SSE connection establishment response to " +
      std::to_string(client_fd) + "...");
  send(client_fd, response.c_str(), response.size(), 0);

  const int check_interrupt_times = update_interval_ * 10;

  while (running_) {
    std::string data = "data: " + GetData() + "\r\n\r\n";
    logger::Logger::Instance().Debug("Sending SSE event to " +
                                     std::to_string(client_fd) + "...");

    if (send(client_fd, data.c_str(), data.size(), MSG_NOSIGNAL) < 0) {
      if (errno == EPIPE || errno == ECONNRESET || errno == EBADF) {
        logger::Logger::Instance().Debug(
            "SSE connection interrupted by client " +
            std::to_string(client_fd));
        break;
      }
    }

    for (int i = 0; i < check_interrupt_times && running_; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

} // namespace webtop::server
