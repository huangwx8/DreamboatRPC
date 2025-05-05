#include <common/Sysutils.hh>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iostream>

using namespace std;

// Function to retrieve the CPU usage
float get_cpu_usage() {
    ifstream file("/proc/stat");
    string line;
    getline(file, line);  // Read the first line, which contains CPU statistics

    stringstream ss(line);
    string cpu;
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    // Total time spent by the CPU
    long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long long idle_time = idle + iowait;

    // CPU usage formula: (total - idle) / total * 100
    static long long prev_total = total, prev_idle = idle_time;
    long long total_diff = total - prev_total;
    long long idle_diff = idle_time - prev_idle;

    prev_total = total;
    prev_idle = idle_time;

    return (float)(total_diff - idle_diff) / (total_diff + 1e5) * 100.0f;
}

// Function to retrieve the memory usage
float get_memory_usage() {
    ifstream file("/proc/meminfo");
    string line;
    long long total_memory = 0, free_memory = 0;

    while (getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            stringstream ss(line);
            ss >> line >> total_memory;
        } else if (line.find("MemFree:") == 0) {
            stringstream ss(line);
            ss >> line >> free_memory;
        }
    }

    return ((float)(total_memory - free_memory) / total_memory) * 100.0f;
}
