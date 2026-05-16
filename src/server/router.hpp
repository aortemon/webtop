#pragma once

#include <functional>
#include <map>
#include <string>


namespace webtop::server {

class RequestHandler {
 public:
  virtual ~RequestHandler() = default;
  virtual void Handle(int client_fd, const std::string &path) = 0;
};

using HandlerFunc = std::function<void(int client_fd, const std::string &path)>;

class Router {
 public:
  Router();

  void AddRoute(const std::string &path, HandlerFunc handler);
  void AddRoute(const std::string &path, RequestHandler *handler);

  bool Route(int client_fd, const std::string &path);

  void SetNotFoundHandler(HandlerFunc handler);

 private:
  std::map<std::string, HandlerFunc> routes_;
  HandlerFunc not_found_handler_;
};

} // namespace webtop::server
