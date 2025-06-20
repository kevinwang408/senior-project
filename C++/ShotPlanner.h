// ShotPlanner.h
// ===========================================================================
// Provides functionality to select viable child ball-to-hole shots that are
// not obstructed by other balls on the billiards table.
//
// Key functions:
// - isPathObstructed: checks if a straight path is blocked.
// - selectClearShots: returns all non-blocked child ball-to-hole shots.
// ===========================================================================

#ifndef SHOT_PLANNER_H
#define SHOT_PLANNER_H

#include <vector>

// ---------------------------------------------------------------------------
// Checks if a path from point (x1, y1) to (x2, y2) is obstructed by any
// object in 'obstacles' based on their proximity to the path.
// 
// Each obstacle is treated as a circle with radius 'bound_radius'.
// The function calculates the perpendicular distance from each obstacle to
// the path and compares it to the radius. Also checks that the obstacle is
// within the segment length (not beyond the shot).
//
// Returns true if any obstacle blocks the path; false otherwise.
// ---------------------------------------------------------------------------
bool isPathObstructed(
    double x1, double y1, double x2, double y2,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
);

// ---------------------------------------------------------------------------
// Iterates over all combinations of cueball (or childballs) and holes,
// returning a list of valid (child ball, hole) pairs that are not obstructed
// by any other balls.
//
// This function is used to build a candidate list of possible direct shots.
//
// Arguments:
// - cueballs: positions of child balls (usually same as obstacles)
// - holes: positions of holes
// - obstacles: list of all balls to consider as obstructions
// - bound_radius: collision margin (e.g., ball diameter)
//
// Returns a list of pairs where each pair = (child ball position, hole position)
// ---------------------------------------------------------------------------
std::vector<std::pair<std::vector<double>, std::vector<double>>> selectClearShots(
    const std::vector<std::vector<double>>& cueballs,
    const std::vector<std::vector<double>>& holes,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
);

#endif // SHOT_PLANNER_H
