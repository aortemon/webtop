#pragma once

#include <vector>

#include "aggregator/common_snapshot.hpp"


namespace webtop::aggregator {

class DiskBuilder {
 public:
  static std::vector<DiskSnapshot> Build();
};

} // namespace webtop::aggregator
