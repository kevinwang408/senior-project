#include "FileIOUtils.h"
#include <fstream>
#include <sstream>

std::vector<std::vector<double>> loadCSV2D(const std::string& path, int expected_cols) {
    std::ifstream file(path);
    std::vector<std::vector<double>> data;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<double> row;
        std::string value;
        while (std::getline(ss, value, ',')) {
            row.push_back(std::stod(value));
        }
        if (row.size() == expected_cols) {
            data.push_back(row);
        }
    }
    return data;
}

int loadSingleInt(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    int value = 0;
    while (std::getline(file, line)) {
        value = std::stoi(line);
    }
    return value;
}
