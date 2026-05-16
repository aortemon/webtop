#include "http_server.hpp"

#include <array>
#include <asm-generic/socket.h>
#include <bits/types.h>
#include <bits/types/sigset_t.h>
#include <cerrno>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h> // NOLINT(modernize-deprecated-headers)
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include "aggregator/aggregator.hpp"
#include "logger/logger.hpp"
#include "server/handlers/metrics_handler.hpp"
#include "server/handlers/process_handler.hpp"
#include "server/handlers/static_handler.hpp"

namespace webtop::server {

HttpServer::HttpServer(aggregator::MetricsAggregator &aggregator, int port,
                       std::string static_dir, int common_update_rate,
                       int procs_update_rate)
    : common_update_rate_(common_update_rate),
      procs_update_rate_(procs_update_rate), port_(port), server_fd_(-1),
      static_root_(std::move(static_dir)), running_(false),
      aggregator_(aggregator) {
  SetupRoutes();
}

HttpServer::~HttpServer() { Stop(); }

void HttpServer::SetupRoutes() {
  metrics_handler_ = std::make_unique<MetricsHandler>(aggregator_, running_,
                                                      common_update_rate_);
  processes_handler_ = std::make_unique<ProcessesHandler>(aggregator_, running_,
                                                          procs_update_rate_);
  static_handler_ = std::make_unique<StaticHandler>(static_root_);
  router_.AddRoute("/api/dashboard", [this](int fd, const std::string &path) {
    metrics_handler_->Handle(fd, path);
  });

  router_.AddRoute("/api/procs", [this](int fd, const std::string &path) {
    processes_handler_->Handle(fd, path);
  });

  router_.AddRoute("/", [this](int fd, const std::string & /*path*/) {
    static_handler_->Handle(fd, "/procs.html");
  });

  router_.AddRoute("/dashboard.html", [this](int fd, const std::string &path) {
    static_handler_->Handle(fd, path);
  });

  router_.AddRoute("/procs.html", [this](int fd, const std::string &path) {
    static_handler_->Handle(fd, path);
  });

  router_.AddRoute("/css/dashboard.css",
                   [this](int fd, const std::string &path) {
                     static_handler_->Handle(fd, path);
                   });

  router_.AddRoute("/css/procs.css", [this](int fd, const std::string &path) {
    static_handler_->Handle(fd, path);
  });

  router_.AddRoute("/js/dashboard.js", [this](int fd, const std::string &path) {
    static_handler_->Handle(fd, path);
  });

  router_.AddRoute("/js/procs.js", [this](int fd, const std::string &path) {
    static_handler_->Handle(fd, path);
  });

  router_.SetNotFoundHandler([](int client_fd, const std::string & /*path*/) {
    std::string response = "HTTP/1.1 404 Not Found\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: 48\r\n\r\n";
    response += "<html><body><h1>404 Not Found</h1></body></html>";
    send(client_fd, response.c_str(), response.size(), 0);
    close(client_fd);
  });
}

void HttpServer::HandleClient(int client_fd) {
  // If client descriptor is not be closed to some reason,
  // it will be done automatically in the end of HandleClient method
  FDCloseGuard guard(client_fd);

  constexpr int kRequestBuffSize = 4096;
  std::array<char, kRequestBuffSize> buff{};
  auto n = read(client_fd, buff.data(), buff.size() - 1);

  if (n <= 0) {
    close(client_fd);
    return;
  }

  buff.at(n) = '\0';
  std::string request(buff.data());

  size_t method_end = request.find(' ');
  if (method_end == std::string::npos) {
    close(client_fd);
    return;
  }

  std::string method = request.substr(0, method_end);

  size_t path_start = method_end + 1;
  size_t path_end = request.find(' ', path_start);
  if (path_end == std::string::npos) {
    close(client_fd);
    return;
  }

  std::string path = request.substr(path_start, path_end - path_start);

  if (method != "GET") {
    std::string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    logger::Logger::Instance().Error("405\t" + method +
                                     "\t Method not allowed");
    send(client_fd, response.c_str(), response.size(), 0);
    close(client_fd);
    return;
  }

  router_.Route(client_fd, path);
}

void HttpServer::HandleClientAsync(int client_fd) {
  try {
    std::thread([this, client_fd]() { HandleClient(client_fd); }).detach();
  } catch (const std::system_error &e) {
    logger::Logger::Instance().Error("Failed to create thread: " +
                                     std::string(e.what()));
    close(client_fd);
  }
}

void HttpServer::PrintStartMessage() {
  logger::Logger::Instance().Info("Starting WebTop server...");
  logger::Logger::Instance().Debug("Current configuration:");
  logger::Logger::Instance().Debug("Port: " + std::to_string(port_));
  logger::Logger::Instance().Debug("Static dir root: " + static_root_);
  logger::Logger::Instance().Debug("Update rate coomon: " +
                                   std::to_string(common_update_rate_));
  logger::Logger::Instance().Debug("Update rate procs: " +
                                   std::to_string(procs_update_rate_));
  logger::Logger::Instance().EmptyLine();
  logger::Logger::Instance().Debug("Server PID: " + std::to_string(getpid()));
}

void HttpServer::AcceptLoop() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);

  while (running_) {
    struct pollfd pfd{.fd = server_fd_, .events = POLLIN, .revents = 0};

    struct timespec timeout{};
    timeout.tv_nsec = static_cast<__syscall_slong_t>(500 * 1'000'000);
    timeout.tv_sec = 0;

    int ret = ppoll(&pfd, 1, &timeout, &mask);

    if (!running_) {
      logger::Logger::Instance().Debug("Stopping connection accept loop...");
      break;
    }

    if (ret < 0) {
      logger::Logger::Instance().Debug(
          "AcceptionLoop: ppoll returned -1 with errno " +
          std::to_string(errno) + ", ignoring...");
      continue;
    }

    if (ret == 0) {
      continue;
    }

    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd_, reinterpret_cast<struct sockaddr *>(&client_addr),
               &client_len);

    if (client_fd < 0) {
      continue;
    }

    HandleClientAsync(client_fd);
  }
}

void HttpServer::Start() {
  PrintStartMessage();
  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    return;
  }

  int opt = 1;
  setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);

  logger::Logger::Instance().EmptyLine();
  logger::Logger::Instance().Info(
      "Starting server on http://localhost:" + std::to_string(port_) + "...");

  if (bind(server_fd_, reinterpret_cast<struct sockaddr *>(&address),
           sizeof(address)) < 0) {
    close(server_fd_);
    return;
  }

  if (listen(server_fd_, 10) < 0) {
    close(server_fd_);
    return;
  }

  running_ = true;

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &mask, nullptr);

  accept_thread_ = std::thread(&HttpServer::AcceptLoop, this);

  pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);
}

void HttpServer::Stop() {
  running_ = false;

  if (server_fd_ >= 0) {
    shutdown(server_fd_, SHUT_RDWR);
    close(server_fd_);
    server_fd_ = -1;
  }

  if (accept_thread_.joinable()) {
    accept_thread_.join();
  }
}

} // namespace webtop::server
