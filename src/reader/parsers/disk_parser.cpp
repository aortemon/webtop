#include "disk_parser.hpp"

#include <algorithm>
#include <sys/statvfs.h>

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "reader/file_reader.hpp"

namespace webtop::reader {

MountEntry DiskReader::ParseMountLine(const std::string &line) {
  MountEntry entry;
  std::istringstream iss(line);

  iss >> entry.device >> entry.mount_point >> entry.fs_type >> entry.options;

  return entry;
}

bool DiskReader::IsVirtualFilesystem(const std::string &fs_type) {
  static const std::vector<std::string> kVirtualFs = {
      "tmpfs",    "devtmpfs",  "proc",        "sysfs",    "cgroup",
      "cgroup2",  "devpts",    "securityfs",  "fusectl",  "pstore",
      "bpf",      "autofs",    "binfmt_misc", "configfs", "debugfs",
      "tracefs",  "hugetlbfs", "mqueue",      "nfsd",     "overlay",
      "squashfs", "ramfs"};

  return (std::ranges::any_of(
      kVirtualFs, [&fs_type](const auto &vfs) { return fs_type == vfs; }));
}

bool DiskReader::IsPhysicalDisk(const std::string &device) {
  if (device.starts_with("/dev/loop")) {
    return false;
  }

  if (device.starts_with("/dev/ram")) {
    return false;
  }

  if (device.starts_with("/dev/sd")) {
    return true;
  }
  if (device.starts_with("/dev/nvme")) {
    return true;
  }
  if (device.starts_with("/dev/vd")) {
    return true;
  }
  if (device.starts_with("/dev/xvd")) {
    return true;
  }
  if (device.starts_with("/dev/mmcblk")) {
    return true;
  }

  return false;
}

bool DiskReader::IsRealDevice(const std::string &device,
                              const std::string &fs_type) {
  if (device.empty() || device == "none") {
    return false;
  }

  if (IsVirtualFilesystem(fs_type)) {
    return false;
  }

  if (!IsPhysicalDisk(device)) {
    return false;
  }

  return true;
}

bool DiskReader::GetStatsByPath(const std::string &path, DiskStats &stats) {
  struct statvfs stat{};

  if (statvfs(path.c_str(), &stat) != 0) {
    return false;
  }

  uint64_t block_size = stat.f_frsize;
  stats.total_bytes = stat.f_blocks * block_size;
  stats.free_bytes = stat.f_bfree * block_size;
  stats.available_bytes = stat.f_bavail * block_size;
  stats.used_bytes = stats.total_bytes - stats.free_bytes;

  if (stats.total_bytes > 0) {
    stats.used_percent =
        100.0 * static_cast<double>(stats.used_bytes) / static_cast<double>(stats.total_bytes);
  } else {
    stats.used_percent = 0.0;
  }

  return true;
}

std::vector<DiskStats> DiskReader::ReadAll() {
  std::vector<DiskStats> disks;
  auto lines = FileReader::ReadLines("/proc/mounts");

  for (const auto &line : lines) {
    if (line.empty()) {
      continue;
    }

    auto entry = ParseMountLine(line);

    if (!IsRealDevice(entry.device, entry.fs_type)) {
      continue;
    }

    DiskStats stats;
    stats.device = entry.device;
    stats.mount_point = entry.mount_point;
    stats.fs_type = entry.fs_type;

    if (GetStatsByPath(entry.mount_point, stats)) {
      disks.push_back(stats);
    }
  }

  return disks;
}

} // namespace webtop::reader
