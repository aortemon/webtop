#pragma once

namespace webtop::reader {

struct UptimeStats {
  double uptime_seconds;
  double idle_seconds;
};

class UptimeParser {
public:
  static UptimeStats Read();
};

} // namespace webtop::reader
