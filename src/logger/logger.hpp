#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>

namespace webtop::logger {

constexpr std::string_view kReset = "\033[0m";
constexpr std::string_view kGreen = "\033[32m";
constexpr std::string_view kRed = "\033[31m";
constexpr std::string_view kYellow = "\033[33m";
constexpr std::string_view kCyan = "\033[36m";

class Logger {
  enum Level : uint8_t { kDebug, kInfo, kWarning, kError, kSuccess };

  bool debug_printing_ = false;
  bool colored_output_ = false;
  std::mutex mutex_;

  void Log(Level level, const std::string &text);
  static std::string Timestamp();

public:
  static Logger &Instance();
  void SetDebugPrinting(bool debug_printing);
  void SetColoredOutput(bool colored_output);
  void Debug(const std::string &text);
  void Info(const std::string &text);
  void Warning(const std::string &text);
  void Error(const std::string &text);
  void Success(const std::string &text);
  void EmptyLine();
};

} // namespace webtop::logger
