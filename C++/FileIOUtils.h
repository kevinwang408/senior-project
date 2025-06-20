#ifndef FILE_IO_UTILS_H
#define FILE_IO_UTILS_H

#include <vector>
#include <string>

// Load 2D coordinates from a CSV file
std::vector<std::vector<double>> loadCSV2D(const std::string& path, int expected_cols);

// Load a single integer value from a CSV file
int loadSingleInt(const std::string& path);

#endif // FILE_IO_UTILS_H
