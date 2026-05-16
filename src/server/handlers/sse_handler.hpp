#pragma once

#include <atomic>
#include <string>

#include "server/router.hpp"


namespace webtop::server {

class SseHandler : public RequestHandler {
 public:
  SseHandler(std::atomic<bool> &running, int update_interval);
  ~SseHandler() override;

  void Handle(int client_fd, const std::string &path) override;
  virtual std::string GetData() = 0;

 private:
  std::atomic<bool> &running_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
  int update_interval_;
};

} // namespace webtop::server
