#include "file_reader.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "logger/logger.hpp"

namespace webtop::reader {

std::ifstream FileReader::OpenFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    logger::Logger::Instance().Warning("Can't open file " + path);
  }
  return file;
}

std::vector<std::string>
FileReader::SplitByWhitespaceImpl(const std::string &str) {
  std::vector<std::string> splitted;
  std::istringstream iss(str);
  std::string item;

  while (iss >> item) {
    splitted.push_back(item);
  }
  return splitted;
}

std::vector<std::string>
FileReader::SplitByDelimeterImpl(const std::string &str, char delimiter) {
  std::vector<std::string> splitted;
  std::stringstream ss(str);
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    if (!item.empty()) {
      splitted.push_back(item);
    }
  }
  return splitted;
}

std::string FileReader::ReadFile(const std::string &path) {
  std::ifstream file = OpenFile(path);
  if (!file.is_open()) {
    logger::Logger::Instance().Warning("Can't open file " + path);
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::vector<std::string> FileReader::ReadLines(const std::string &path) {
  std::ifstream file = OpenFile(path);
  if (!file.is_open()) {
    return {};
  }
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }
  return lines;
}

std::vector<std::string> FileReader::Split(const std::string &str,
                                           char delimiter) {
  if (delimiter == ' ') {
    return SplitByWhitespaceImpl(str);
  }
  return SplitByDelimeterImpl(str, delimiter);
}

} // namespace webtop::reader
