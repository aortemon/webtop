#pragma once

#include <string>

#include "server/router.hpp"

namespace webtop::server {

class StaticHandler : public RequestHandler {
public:
  explicit StaticHandler(std::string static_root);

  void Handle(int client_fd, const std::string &path) override;

private:
  std::string static_root_;

  static std::string ReadFile(const std::string &path);
  static std::string GetMimeType(const std::string &path);
  static void SendResponse(int client_fd, int status_code,
                           const std::string &mime, const std::string &content);
  static void Send404(int client_fd);
};

} // namespace webtop::server
