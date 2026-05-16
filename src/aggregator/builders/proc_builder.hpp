#pragma once

#include "aggregator/common_snapshot.hpp"


namespace webtop::aggregator {

class ProcBuilder {
 public:
  static FullProcessSnapshot Build();
};

} // namespace webtop::aggregator
