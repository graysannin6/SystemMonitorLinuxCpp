#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "linux_parser.h"

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
  string os, kernel, version;
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
float LinuxParser::MemoryUtilization() 
{  
  std::string line, key;
  float memTotal{},memFree,buffers{};

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream,line))
    {
      std::istringstream linestream(line);
      linestream>>key;
      if (key == "MemTotal:")
      {
        linestream>>memTotal;
      }
      else if (key == "MemFree:")
      {
        linestream>>memFree;
      }
      else if (key == "Buffers:")
      {
        linestream>>buffers;
        break;
      }
    
    }
    
  }
  return (memTotal - memFree - buffers) / memTotal;
  
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() 
{  
  long uptime{};
  std::string line;
  std::ifstream stream (kProcDirectory + kUptimeFilename);
  if (stream.is_open())
  {
    std::getline(stream,line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() 
{ 
  long jiffies{},value{};
  std::string line,key;
  //open the path
  std::ifstream stream(kProcDirectory + kStatFilename);

  //check if file is open
  if (stream.is_open())
  {
    std::getline(stream,line);
    std::istringstream linestream(line);

    //read cpu prefix
    linestream >> key;

    //check if prefix matches
    if (key == "cpu")
    {
      while (linestream >> value)
      {
        jiffies += value;
      }
      
    }
    
  }
  
  return jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) 
{ 
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  long activeJiffies{};
  if (filestream.is_open())
  {
    std::string line,value;
    std::getline(filestream,line);
    std::istringstream linestream(line);
    int count = 1;
    while (linestream>>value)
    {
      if (count == 14 || count == 15 ||count == 16 || count == 17)
      {
        activeJiffies += std::stol(value);
      }
      count++;
    }
    
  }
  return activeJiffies;
  
}


long LinuxParser::ActiveJiffies() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  long total = 0;
  if (stream.is_open()) {
    std::string line, value;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value; // Skip the "cpu" prefix
    
    // Now read each time slice
    while (linestream >> value) {
      if (value != "cpu") {
        total += std::stol(value);
      }
    }
  }
  return total;
}


// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  long idle = 0, iowait = 0;
  if (stream.is_open()) {
    std::string line, value;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value; // Skip the "cpu" prefix
    
    // Iterate through the fields to get to the idle and iowait values
    for (int i = 0; i < 4 && linestream >> value; i++) {
      if (i == 3) { idle = std::stol(value); }
      if (i == 4) { iowait = std::stol(value); }
    }
  }
  return idle + iowait;
}


// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::vector<string> cpu_values;
  if (stream.is_open()) {
    std::string line;
    while (std::getline(stream, line)) {
      if (line.substr(0, 3) == "cpu") {
        std::istringstream linestream(line);
        std::vector<std::string> values(
          std::istream_iterator<std::string>{linestream},
          std::istream_iterator<std::string>()
        );
        cpu_values.insert(cpu_values.end(), values.begin() + 1, values.end());
      }
    }
  }
  return cpu_values;
}


/// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  int total = 0;
  bool found = false;

  if (!stream) {
    std::cerr << "Unable to open " << kProcDirectory + kStatFilename << std::endl;
    return 0; // Or throw an exception, depending on how you want to handle this error
  }

  while (std::getline(stream, line)) {
    if (line.substr(0, 9) == "processes") {
      std::istringstream linestream(line);
      std::string label;
      if (linestream >> label >> total) {
        found = true;
        break;
      }
    }
  }

  stream.close();

  if (!found) {
    std::cerr << "'processes' entry not found in " << kProcDirectory + kStatFilename << std::endl;
    return 0; // Or handle this case appropriately
  }

  return total;
}


// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  int running = 0;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.substr(0, 13) == "procs_running") {
        std::istringstream linestream(line);
        std::string label;
        linestream >> label >> running;
        break;
      }
    }
  }
  return running;
}


// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) 
{
  std::string command{};
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open())
  {
    std::getline(stream,command);
  }
   
  return command; 
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::string line;
  std::string key;
  long value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:") {
        // Convert from kB to MB
        value /= 1024;
        return std::to_string(value);
      }
    }
  }
  return string();
}


// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return string();
}


// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  std::string uid = Uid(pid); // Fetch the uid using previously defined function
  std::string line;
  std::string user;
  std::string x; // Placeholder for 'x' in the passwd file
  std::string uid_read;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> x >> uid_read) {
        if (uid_read == uid) {
          return user;
        }
      }
    }
  }
  return string();
}


// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::string line;
  std::string value;
  long start_time = 0;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::vector<std::string> values(
      std::istream_iterator<std::string>{linestream},
      std::istream_iterator<std::string>()
    );

    // According to procfs, the 22nd value is the starttime for the process
    if (values.size() > 21) {
      start_time = std::stol(values[21]);
      // The uptime of the process is the difference between the system uptime and the process start time
      long uptime = UpTime() - (start_time / sysconf(_SC_CLK_TCK));
      return uptime;
    }
  }
  return 0;
}