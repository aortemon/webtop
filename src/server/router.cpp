#include "router.hpp"

#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>


namespace webtop::server {

Router::Router() {
  not_found_handler_ = [](int client_fd, const std::string & /*path*/) {
    std::string response = "HTTP/1.1 404 Not Found\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: 48\r\n\r\n";
    response += "<html><body><h1>404 Not Found</h1></body></html>";
    send(client_fd, response.c_str(), response.size(), 0);
    close(client_fd);
  };
}

void Router::AddRoute(const std::string &path, HandlerFunc handler) {
  routes_[path] = std::move(handler);
}

void Router::AddRoute(const std::string &path, RequestHandler *handler) {
  routes_[path] = [handler](int client_fd, const std::string &path) {
    handler->Handle(client_fd, path);
  };
}

bool Router::Route(int client_fd, const std::string &path) {
  auto it = routes_.find(path);
  if (it != routes_.end()) {
    it->second(client_fd, path);
    return true;
  }

  if (not_found_handler_) {
    not_found_handler_(client_fd, path);
  }
  return false;
}

void Router::SetNotFoundHandler(HandlerFunc handler) {
  not_found_handler_ = std::move(handler);
}

} // namespace webtop::server
