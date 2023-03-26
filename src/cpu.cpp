#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/bind/bind.hpp>

#include "cpu.hpp"

#define CPU_STAT_FIELDS 11
#define CPU_READ_INTERVAL 200

// Reads from the stat file for cpu info, and parses that info into a vector.
std::vector<std::array<int, 10>> parse_cpu_info(std::ifstream &&stat_file) {
  std::vector<std::array<int, 10>> results;
  std::string line;
  std::vector<std::string> fields;
  std::array<int, 10> values;
  // For each line in the stat_file
  while (std::getline(stat_file, line)) {
    // Split the line based on spaces
    boost::split(fields, line, boost::is_any_of(" "), boost::token_compress_on);
    // Trim out any additional whitespace
    std::for_each(fields.begin(), fields.end(),
                  boost::bind(&boost::trim<std::string>,
                              boost::placeholders::_1, std::locale()));

    // If the first field in the split line has "cpu"
    if (fields.size() == CPU_STAT_FIELDS &&
        fields[0].find("cpu") != std::string::npos) {
      // Collect and parse the remaining fields
      auto numeric_fields =
          std::vector<std::string>(fields.begin() + 1, fields.end());
      for (int i = 1; i < CPU_STAT_FIELDS; i++) {
        values[i - 1] = std::stoi(numeric_fields[i - 1]);
      }
      // Then add to the results
      results.push_back(values);
    }
  }
  stat_file.close();
  return results;
}

// Provides a vector containing work time and total time pairs.
std::vector<std::pair<int, int>> get_cpu_jiffies() {
  // Open the stat file and pass it to the parse_cpu_info function
  std::ifstream stat_file("/proc/stat");
  if (!stat_file.is_open()) {
    std::cerr << "Failed to open /proc/stat for cpu information" << std::endl;
    exit(1);
  }
  auto raw_values = parse_cpu_info(std::move(stat_file));

  // Calculate work_jiffies and the total_jiffies, then add them to results
  std::vector<std::pair<int, int>> results;
  for (auto vals : raw_values) {
    auto total_jiffies = std::accumulate(vals.cbegin(), vals.cend(), 0);
    auto work_jiffies = vals[0] + vals[1] + vals[2];
    auto usage_pair = std::make_pair(work_jiffies, total_jiffies);
    results.push_back(usage_pair);
  }
  return results;
}

// Provides the current cpu usage. The numbers are simple proportions, not
// percentages.
std::vector<double> get_cpu_usage() {
  // To find usage, we need to compare time spent working vesus total time.
  // I had come across a post describing the math behind it, but I can't find it
  // anymore. I'll link to it if I can find it again.
  auto jiffies_start = get_cpu_jiffies();
  std::this_thread::sleep_for(std::chrono::milliseconds{CPU_READ_INTERVAL});
  auto jiffies_end = get_cpu_jiffies();

  // Apply some math to find usages
  std::vector<double> results;
  for (int i = 0; i < jiffies_start.size(); i++) {
    auto [work_jiffies_start, total_jiffies_start] = jiffies_start[i];
    auto [work_jiffies_end, total_jiffies_end] = jiffies_end[i];
    double work_over_period = work_jiffies_end - work_jiffies_start;
    double total_over_period = total_jiffies_end - total_jiffies_start;
    double usage = work_over_period / total_over_period * 100;
    results.push_back(usage);
  }
  return results;
}
