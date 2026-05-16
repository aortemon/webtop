#pragma once

#include "aggregator/aggregator.hpp"
#include "server/handlers/sse_handler.hpp"
#include <atomic>
#include <string>


namespace webtop::server {

class MetricsHandler : public SseHandler {
  aggregator::MetricsAggregator &aggregator_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
  std::string GetData() override;

 public:
  explicit MetricsHandler(aggregator::MetricsAggregator &aggregator, std::atomic<bool> &running,
                          int update_interval);
  ~MetricsHandler() override;
};

} // namespace webtop::server
