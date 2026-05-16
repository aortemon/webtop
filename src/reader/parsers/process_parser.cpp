#include "reader/parsers/process_parser.hpp"

#include <dirent.h>
#include <pwd.h>

#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "reader/file_reader.hpp"
#include "util/scope_guard.hpp"

namespace webtop::reader {

int ProcessReader::FilterProcDirs(const struct dirent *entry) {
  const char *name = static_cast<const char *>(entry->d_name);

  if (name[0] < '0' || name[0] > '9') {
    return 0;
  }

  for (int i = 0; name[i] != '\0'; ++i) {
    if (name[i] < '0' || name[i] > '9') {
      return 0;
    }
  }
  return 1;
}

int32_t ProcessReader::ParseUidFromStatus(const std::string &status_content) {
  std::istringstream iss(status_content);
  std::string line;

  while (std::getline(iss, line)) {
    if (line.starts_with("Uid:")) {
      std::istringstream line_iss(line);
      std::string label;
      int32_t real_uid = 0;
      int32_t effective_uid = 0;
      int32_t saved_uid = 0;
      int32_t fs_uid = 0;

      line_iss >> label >> real_uid >> effective_uid >> saved_uid >> fs_uid;
      return real_uid;
    }
  }

  return -1;
}

std::string ProcessReader::ReadStatusFile(int32_t pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/status";
  return FileReader::ReadFile(path);
}

ProcessInfo ProcessReader::ParseStatFile(int32_t pid,
                                         const std::string &content) {
  ProcessInfo info = {};
  info.pid = pid;
  info.ppid = 0;
  info.uid = -1;

  size_t open_paren = content.find('(');
  size_t close_paren = content.rfind(')');

  if (open_paren == std::string::npos || close_paren == std::string::npos ||
      close_paren <= open_paren) {
    return info;
  }

  info.name = content.substr(open_paren + 1, close_paren - open_paren - 1);

  std::string rest = content.substr(close_paren + 2);

  std::istringstream iss(rest);

  iss >> info.state;
  iss >> info.ppid;

  // Ignoring:
  // pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajfkt,
  std::string tmp;
  for (int i = 0; i < 9; ++i) {
    iss >> tmp;
  }

  iss >> info.utime;
  iss >> info.stime;
  iss >> info.cutime;
  iss >> info.cstime;
  iss >> info.priority;
  iss >> info.nice;
  iss >> info.num_threads;

  // Ignoring:
  // itrealvalue
  iss >> tmp;

  iss >> info.starttime_ticks;
  iss >> info.vmem_bytes;
  iss >> info.rss_pages;

  return info;
}

std::string ProcessReader::ReadStatFile(int32_t pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/stat";
  return FileReader::ReadFile(path);
}

std::vector<int32_t> ProcessReader::GetAllPids() {
  std::vector<int32_t> pids;

  struct dirent **namelist = nullptr;
  int count = scandir("/proc", &namelist, FilterProcDirs, alphasort);

  auto scope_guard = util::MakeScopeGuard([namelist, count]() {
    if (namelist) {
      for (int i = 0; i < count; i++) {
        util::Free(namelist[i]);
      }
      util::Free(namelist);
    }
  });

  if (count < 0) {
    return pids;
  }

  pids.reserve(count);

  for (int i = 0; i < count; ++i) {
    int32_t pid = std::stoi(static_cast<char *>(namelist[i]->d_name));
    pids.push_back(pid);
  }
  return pids;
}

int32_t ProcessReader::GetProcessCount() {
  struct dirent **namelist = nullptr;
  int count = scandir("/proc", &namelist, FilterProcDirs, nullptr);

  if (count < 0) {
    return 0;
  }

  auto scope_guard = util::MakeScopeGuard([namelist, count]() {
    if (namelist) {
      for (int i = 0; i < count; i++) {
        util::Free(namelist[i]);
      }
      util::Free(namelist);
    }
  });

  return count;
}

int32_t ProcessReader::GetThreadCount() {
  int32_t total_threads = 0;
  auto pids = GetAllPids();

  for (int32_t pid : pids) {
    std::string task_path = "/proc/" + std::to_string(pid) + "/task";

    struct dirent **task_list = nullptr;
    int task_count =
        scandir(task_path.c_str(), &task_list, FilterProcDirs, nullptr);

    auto scope_guard = util::MakeScopeGuard([task_list, task_count]() {
      if (task_list) {
        for (int i = 0; i < task_count; i++) {
          util::Free(task_list[i]);
        }
        util::Free(task_list);
      }
    });

    if (task_count > 0) {
      total_threads += task_count;
    }
  }

  return total_threads;
}

std::string ProcessReader::GetUsernameByUid(int32_t uid) {
  struct passwd pwd{};
  struct passwd *result = nullptr;
  std::array<char, 1024> buff{};
  int ret = getpwuid_r(uid, &pwd, buff.data(), buff.size(), &result);
  if (ret == 0 && result != nullptr) {
    return {pwd.pw_name};
  }
  return {};
}

ProcessInfo ProcessReader::GetProcessInfo(int32_t pid) {
  std::string stat_content = ReadStatFile(pid);
  if (stat_content.empty()) {
    return ProcessInfo{};
  }

  ProcessInfo info = ParseStatFile(pid, stat_content);

  std::string status_content = ReadStatusFile(pid);
  if (!status_content.empty()) {
    info.uid = ParseUidFromStatus(status_content);
    info.user = GetUsernameByUid(info.uid);
  }

  return info;
}

std::vector<ProcessInfo> ProcessReader::ReadAll() {
  std::vector<ProcessInfo> processes;
  auto pids = GetAllPids();

  processes.reserve(pids.size());

  for (int32_t pid : pids) {
    ProcessInfo info = GetProcessInfo(pid);
    if (info.pid != 0) {
      processes.push_back(info);
    }
  }

  return processes;
}

} // namespace webtop::reader
