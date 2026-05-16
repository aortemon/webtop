#include "dotenv_parser.hpp"

#include <algorithm>
#include <map>
#include <string>

#include "reader/file_reader.hpp"


namespace webtop::reader {

bool DotEnvParser::IsEmpty(const std::string &line) {
  return std::ranges::all_of(line, [](const auto &x) { return isspace(x); });
}

bool DotEnvParser::IsComment(const std::string &line) {
  return line.starts_with('#');
}

std::map<std::string, std::string> DotEnvParser::ReadAll(const std::string &path) {
  auto lines = FileReader::ReadLines(path);
  std::map<std::string, std::string> dotenv;
  for (const auto &line : lines) {
    if (IsEmpty(line) || IsComment(line)) {
      continue;
}
    if (std::ranges::count(line, '=') != 1) {
      continue;
    }
    auto splitted = FileReader::Split(line, '=');
    dotenv[splitted[0]] = splitted[1];
  }
  return dotenv;
}
} // namespace webtop::reader
