#pragma once

#include "aggregator/aggregator.hpp"
#include "server/handlers/sse_handler.hpp"
#include <atomic>
#include <string>


namespace webtop::server {

class ProcessesHandler : public SseHandler {
  aggregator::MetricsAggregator &aggregator_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
  std::string GetData() override;

 public:
  explicit ProcessesHandler(aggregator::MetricsAggregator &aggregator, std::atomic<bool> &running,
                            int update_interval);
  ~ProcessesHandler() override;
};

} // namespace webtop::server
