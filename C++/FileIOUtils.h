// FileIOUtils.h
// ===========================================================================
// This header defines utility functions to read CSV-based coordinate data
// from disk, including:
// - 2D double coordinates (e.g., cue ball, child ball positions)
// - Single integer values (e.g., number of balls)
//
// Used for input parsing in the billiards planning pipeline.
// ===========================================================================

#ifndef FILE_IO_UTILS_H
#define FILE_IO_UTILS_H

#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// Loads a list of 2D coordinate points (x, y) from a CSV file.
// Each row must contain exactly 'expected_cols' number of numeric entries.
// Example input line: 152.3,98.7
// Returns a vector of vectors where each inner vector represents a coordinate.
// ---------------------------------------------------------------------------
std::vector<std::vector<double>> loadCSV2D(const std::string& path, int expected_cols);

// ---------------------------------------------------------------------------
// Loads a single integer value from a CSV file.
// Typically used to read ball count or configuration parameter.
// If multiple lines exist, only the last valid integer is returned.
// ---------------------------------------------------------------------------
int loadSingleInt(const std::string& path);

#endif // FILE_IO_UTILS_H
