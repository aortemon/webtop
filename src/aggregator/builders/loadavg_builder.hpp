#pragma once

#include "aggregator/common_snapshot.hpp"


namespace webtop::aggregator {

class LoadAvgBuilder {
 public:
  static LoadAvgSnapshot Build();
};

} // namespace webtop::aggregator
