#pragma once
#include <map>
#include <string>


namespace webtop::reader {

class DotEnvParser {
  static bool IsEmpty(const std::string &line);
  static bool IsComment(const std::string &line);

 public:
  static std::map<std::string, std::string> ReadAll(const std::string &path);
};

} // namespace webtop::reader
