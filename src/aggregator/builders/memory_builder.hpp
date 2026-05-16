#pragma once

#include "aggregator/common_snapshot.hpp"


namespace webtop::aggregator {

class MemoryBuilder {
 public:
  static MemorySnapshot Build();
};

} // namespace webtop::aggregator
