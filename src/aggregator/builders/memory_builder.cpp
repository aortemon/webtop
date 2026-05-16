#include "memory_builder.hpp"

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/memory_parser.hpp"


namespace webtop::aggregator {

MemorySnapshot MemoryBuilder::Build() {
  auto data = reader::MemoryReader::Read();

  MemorySnapshot snapshot{};
  snapshot.total = data.MemTotalMb();
  snapshot.used = data.MemUsedMb();
  snapshot.available = data.MemAvailableMb();
  snapshot.used_percent = data.MemUsedPercent();
  snapshot.swap_total = data.SwapTotalMb();
  snapshot.swap_used = data.SwapTotalMb();
  snapshot.swap_used_percent = data.SwapUsedPercent();
  snapshot.buff_cache = data.mem_buffers_kb + data.mem_cached_kb;

  return snapshot;
}

} // namespace webtop::aggregator
