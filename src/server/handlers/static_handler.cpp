#include "static_handler.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "logger/logger.hpp"

namespace webtop::server {

StaticHandler::StaticHandler(std::string static_root)
    : static_root_(std::move(static_root)) {}

std::string StaticHandler::ReadFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::string StaticHandler::GetMimeType(const std::string &path) {
  if (path == "/") {
    return "text/html";
  }
  if (path.find(".css") != std::string::npos) {
    return "text/css";
  }
  if (path.find(".js") != std::string::npos) {
    return "application/javascript";
  }
  if (path.find(".html") != std::string::npos) {
    return "text/html";
  }
  if (path.find(".json") != std::string::npos) {
    return "application/json";
  }
  if (path.find(".ico") != std::string::npos) {
    return "image/x-icon";
  }
  return "text/plain";
}

void StaticHandler::SendResponse(int client_fd, int status_code,
                                 const std::string &mime,
                                 const std::string &content) {
  std::string response = "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n";
  response += "Content-Type: " + mime + "\r\n";
  response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
  response += "Connection: close\r\n";

  if (mime == "text/html") {
    response += "Cache-Control: no-cache\r\n";
  } else {
    response += "Cache-Control: max-age=3600\r\n";
  }

  response += "\r\n";
  response += content;

  send(client_fd, response.c_str(), response.size(), 0);
}

void StaticHandler::Send404(int client_fd) {
  std::string body = "<h1>404 Not Found</h1>";
  SendResponse(client_fd, 404, "text/html", body);
}

void StaticHandler::Handle(int client_fd, const std::string &path) {
  std::string filepath;

  filepath = static_root_ + path;

  std::string content = ReadFile(filepath);
  if (content.empty()) {
    Send404(client_fd);

    logger::Logger::Instance().Error("404\tGET\t" + path + "\t" +
                                     std::to_string(client_fd));
  } else {
    std::string mime = GetMimeType(path);
    SendResponse(client_fd, 200, mime, content);
    logger::Logger::Instance().Success("200\tGET\t" + path + "\t" +
                                       std::to_string(client_fd));
  }

  close(client_fd);
}

} // namespace webtop::server
