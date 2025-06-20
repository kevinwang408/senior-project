#ifndef FLIP_PLANNER_H
#define FLIP_PLANNER_H

#include <vector>

// Structure to hold flip shot plan
struct FlipShot {
    std::vector<double> cue_to_wall_vector;
    std::vector<double> wall_contact_point;
    std::vector<double> wall_to_target_vector;
    std::vector<double> target_coords;
    std::vector<double> hole_coords;
    double total_distance;
};

// Evaluate possible flip shots based on wall reflections
std::vector<FlipShot> evaluateFlipShots(
    const std::vector<double>& cueball_pos,
    const std::vector<std::vector<double>>& candidates,
    const std::vector<std::vector<double>>& obstacles,
    const std::vector<std::vector<double>>& walls,
    double bound_radius
);

#endif // FLIP_PLANNER_H
