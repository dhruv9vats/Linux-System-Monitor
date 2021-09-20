#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float mem_total = 0;
  float mem_free = 0;
  string line;
  string key;
  string value;
  string unit;
  std::ifstream file_stream(kProcDirectory + kMeminfoFilename);

  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);

      line_stream >> key >> value >> unit;

      if (key == "MemTotal") {
        mem_total = std::stof(value);
      } else if (key == "MemFree") {
        mem_free = std::stof(value);
      }

      if (mem_free > 0 && mem_total > 0) {
        return mem_total - mem_free;
      }
    }
  }

  return mem_total - mem_free;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string idle_time;

  string line;
  std::ifstream file_stream(kProcDirectory + kUptimeFilename);
  if (file_stream.is_open()) {
    getline(file_stream, line);
    std::istringstream line_stream(line);
    line_stream >> uptime >> idle_time;
  }

  return static_cast<long>(std::stof(uptime));
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  // string line;
  // std::ifstream file_stream(kProcDirectory + kStatFilename);
  // string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest,
  //     guest_nice;
  // if (file_stream.is_open()) {
  //   while (std::getline(file_stream, line)) {
  //     std::replace(line.begin(), line.end(), ':', ' ');
  //     std::istringstream line_stream(line);

  //     line_stream >> cpu >> user >> nice >> system >> idle >> iowait >> irq
  //     >>
  //         softirq >> steal >> guest >> guest_nice;

  //     if (cpu == "cpu") {
  //       return std::stol(user) + std::stol(nice) + std::stol(system) +
  //              std::stol(idle) + std::stol(iowait) + std::stol(irq) +
  //              std::stol(softirq) + std::stol(steal) + std::stol(guest) +
  //              std::stol(guest_nice);
  //     }
  //   }
  // }

  // return static_cast<long>(0);

  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  long active_time = 0;

  // https://man7.org/linux/man-pages/man5/proc.5.html
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) +
                            kUptimeFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);

    string val;
    for (int i = 1; i <= 15; i++) {
      line_stream >> val;
      if (i == 14 or i == 15) active_time += std::stol(val);
    }
    return active_time / sysconf(_SC_CLK_TCK);
  }
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  string line;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest,
      guest_nice;
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);

      line_stream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
          softirq >> steal >> guest >> guest_nice;

      if (cpu == "cpu") {
        return std::stol(user) + std::stol(nice) + std::stol(system) +
               std::stol(irq) + std::stol(softirq) + std::stol(steal);
      }
    }
  }

  return static_cast<long>(0);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string line;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest,
      guest_nice;
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);

      line_stream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
          softirq >> steal >> guest >> guest_nice;

      if (cpu == "cpu") {
        return std::stol(idle) + std::stol(iowait);
      }
    }
  }

  return static_cast<long>(0);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }
