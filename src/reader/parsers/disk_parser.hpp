#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace webtop::reader {

constexpr uint64_t kBytesInKb = 1024;
constexpr uint64_t kBytesInMb = kBytesInKb * kBytesInKb;

struct DiskStats {
  std::string device;
  std::string mount_point;
  std::string fs_type;

  uint64_t total_bytes;
  uint64_t used_bytes;
  uint64_t free_bytes;
  uint64_t available_bytes;

  double used_percent;

  [[nodiscard]] uint64_t TotalMb() const { return total_bytes / (kBytesInMb); }
  [[nodiscard]] uint64_t UsedMb() const { return used_bytes / (kBytesInMb); }
  [[nodiscard]] uint64_t FreeMb() const { return free_bytes / (kBytesInMb); }
  [[nodiscard]] uint64_t AvailableMb() const {
    return available_bytes / (kBytesInMb);
  }
};

struct MountEntry {
  std::string device;
  std::string mount_point;
  std::string fs_type;
  std::string options;
};

class DiskReader {
  static MountEntry ParseMountLine(const std::string &line);
  static bool GetStatsByPath(const std::string &path, DiskStats &stats);
  static bool IsPhysicalDisk(const std::string &device);
  static bool IsVirtualFilesystem(const std::string &fs_type);
  static bool IsRealDevice(const std::string &device,
                           const std::string &fs_type);

public:
  static std::vector<DiskStats> ReadAll();
};

} // namespace webtop::reader
