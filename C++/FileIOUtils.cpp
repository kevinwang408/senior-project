// FileIOUtils.cpp
// ===========================================================================
// Implements CSV file reading utilities for the billiards robotic system.
//
// - loadCSV2D: extracts 2D coordinate points from each line in the file.
// - loadSingleInt: retrieves the last integer value found in the file.
// ===========================================================================

#include "FileIOUtils.h"
#include <fstream>
#include <sstream>

std::vector<std::vector<double>> loadCSV2D(const std::string& path, int expected_cols) {
    std::ifstream file(path);                // Open CSV file
    std::vector<std::vector<double>> data;   // Stores result as list of (x, y) pairs
    std::string line;

    // Read each line and process it
    while (std::getline(file, line)) {
        std::stringstream ss(line);          // Create a stream from the line
        std::vector<double> row;             // Temporary vector for each row
        std::string value;

        // Parse each comma-separated value in the line
        while (std::getline(ss, value, ',')) {
            row.push_back(std::stod(value)); // Convert string to double
        }

        // Ensure row has the expected number of columns before adding
        if (row.size() == expected_cols) {
            data.push_back(row);
        }
    }

    return data;
}

int loadSingleInt(const std::string& path) {
    std::ifstream file(path);     // Open file
    std::string line;
    int value = 0;

    // Read through all lines, storing only the last integer read
    while (std::getline(file, line)) {
        value = std::stoi(line);  // Convert string to integer
    }

    return value;                 // Return final parsed integer
}
