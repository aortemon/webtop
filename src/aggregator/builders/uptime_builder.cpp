#include "uptime_builder.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

#include "reader/parsers/uptime_parser.hpp"

namespace webtop::aggregator {

constexpr int kSecondsInDay = 24 * 60 * 60;
constexpr int kSecondsInHour = 60 * 60;
constexpr int kSecondsInMinute = 60;
constexpr uint64_t kSNPrintfBufferSize = 64;

std::string UptimeBuilder::Build() {
  auto data = reader::UptimeParser::Read();
  auto seconds = static_cast<int>(data.uptime_seconds);

  int days = seconds / kSecondsInDay;
  int hours = (seconds % kSecondsInDay) / kSecondsInHour;
  int minutes = (seconds % kSecondsInHour) / kSecondsInMinute;

  std::array<char, kSNPrintfBufferSize> buff{};
  snprintf(buff.data(), buff.size(), "%dd %02d:%02d", days, hours, minutes);

  return {buff.data()};
}

} // namespace webtop::aggregator
