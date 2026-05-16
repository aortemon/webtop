#pragma once

#include <dirent.h>

#include <cstdint>
#include <string>
#include <vector>


namespace webtop::reader {

struct ProcessInfo {
  int32_t pid;
  int32_t ppid;
  std::string name;
  char state;
  int32_t uid;
  std::string user;

  uint64_t utime;
  uint64_t stime;
  uint64_t cutime;
  uint64_t cstime;

  uint64_t vmem_bytes;
  uint64_t rss_pages;

  int32_t priority;
  int32_t nice;

  int32_t num_threads;

  uint64_t starttime_ticks;
};

class ProcessReader {
  static ProcessInfo ParseStatFile(int32_t pid, const std::string &content);
  static std::string ReadStatusFile(int32_t pid);
  static int32_t ParseUidFromStatus(const std::string &status_content);
  static int FilterProcDirs(const struct dirent *entry);
  static std::string ReadStatFile(int32_t pid);
  static std::vector<int32_t> GetAllPids();
  static ProcessInfo GetProcessInfo(int32_t pid);
  static std::string GetUsernameByUid(int32_t uid);

 public:
  static std::vector<ProcessInfo> ReadAll();
  static int32_t GetProcessCount();
  static int32_t GetThreadCount();
};

} // namespace webtop::reader
