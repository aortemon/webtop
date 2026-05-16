#include "logger/logger.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <mutex>
#include <string>

namespace webtop::logger {

constexpr int32_t kTimeStrBufferSize = 16;

Logger &Logger::Instance() {
  static Logger instance;
  return instance;
}

void Logger::SetDebugPrinting(bool debug_printing) {
  debug_printing_ = debug_printing;
}

void Logger::SetColoredOutput(bool colored_output) {
  colored_output_ = colored_output;
}

void Logger::Log(Level level, const std::string &text) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (colored_output_) {
    switch (level) {
    case kDebug:
      std::cout << kCyan;
      break;
    case kWarning:
      std::cout << kYellow;
      break;
    case kSuccess:
      std::cout << kGreen;
      break;
    case kError:
      std::cout << kRed;
      break;
    case kInfo:
    default:
      break;
    }
  }
  std::cout << text << "\n";
  if (colored_output_) {
    std::cout << kReset;
  }
}

std::string Logger::Timestamp() {
  static std::mutex local_mutex;
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  auto t = std::chrono::system_clock::to_time_t(now);

  std::lock_guard<std::mutex> lock(local_mutex);
  std::tm tm_buf{};
  std::tm *tm_ptr = localtime_r(&t, &tm_buf);

  std::array<char, kTimeStrBufferSize> buf{};
  std::strftime(buf.data(), sizeof(buf), "%H:%M:%S", tm_ptr);
  return std::string(buf.data()) + "." + std::to_string(ms.count());
}

void Logger::Debug(const std::string &text) {
  if (debug_printing_) {
    Log(Level::kDebug, "[ DEBUG ]\t" + Timestamp() + "\t" + text);
  }
}

void Logger::Info(const std::string &text) {
  Log(Level::kInfo, "[ INFO  ]\t" + Timestamp() + "\t" + text);
}

void Logger::Warning(const std::string &text) {
  Log(Level::kWarning, "[ WARN  ]\t" + Timestamp() + "\t" + text);
}

void Logger::Success(const std::string &text) {
  Log(Level::kSuccess, "[ INFO  ]\t" + Timestamp() + "\t" + text);
}

void Logger::Error(const std::string &text) {
  Log(Level::kError, "[ ERROR ]\t" + Timestamp() + "\t" + text);
}

void Logger::EmptyLine() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::cout << '\n';
}
} // namespace webtop::logger
