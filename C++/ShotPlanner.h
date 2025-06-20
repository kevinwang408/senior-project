#ifndef SHOT_PLANNER_H
#define SHOT_PLANNER_H

#include <vector>

// Check if a line between two points is obstructed by other balls
bool isPathObstructed(
    double x1, double y1, double x2, double y2,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
);

// Select target ball and hole combinations without obstruction
std::vector<std::pair<std::vector<double>, std::vector<double>>> selectClearShots(
    const std::vector<std::vector<double>>& cueballs,
    const std::vector<std::vector<double>>& holes,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
);

#endif // SHOT_PLANNER_H
