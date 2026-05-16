#include "disk_builder.hpp"
#include <vector>

#include "aggregator/common_snapshot.hpp"
#include "reader/parsers/disk_parser.hpp"

namespace webtop::aggregator {

std::vector<DiskSnapshot> DiskBuilder::Build() {
  std::vector<DiskSnapshot> result;
  auto disks = reader::DiskParser::ReadAll();

  for (const auto &disk : disks) {
    DiskSnapshot snapshot;
    snapshot.device = disk.device;
    snapshot.mount_point = disk.mount_point;
    snapshot.total_mb = disk.TotalMb();
    snapshot.used_mb = disk.UsedMb();
    snapshot.free_mb = disk.FreeMb();
    snapshot.used_percent = disk.used_percent;
    result.push_back(snapshot);
  }

  return result;
}

} // namespace webtop::aggregator
