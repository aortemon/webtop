#pragma once
#include <fstream>
#include <string>
#include <vector>


namespace webtop::reader {

class FileReader {
  static std::ifstream OpenFile(const std::string &path);
  static std::vector<std::string> SplitByWhitespaceImpl(const std::string &str);
  static std::vector<std::string> SplitByDelimeterImpl(const std::string &str, char delimiter);

 public:
  static std::string ReadFile(const std::string &path);
  static std::vector<std::string> ReadLines(const std::string &path);

  static std::vector<std::string> Split(const std::string &str, char delimiter = ' ');
};
} // namespace webtop::reader
