#include "loadavg_builder.hpp"

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/loadavg_parser.hpp"

namespace webtop::aggregator {

LoadAvgSnapshot LoadAvgBuilder::Build() {
  auto data = reader::LoadAvgParser::Read();

  LoadAvgSnapshot snapshot{};
  snapshot.load_1min = data.load_1min;
  snapshot.load_5min = data.load_5min;
  snapshot.load_15min = data.load_15min;

  return snapshot;
}

} // namespace webtop::aggregator
