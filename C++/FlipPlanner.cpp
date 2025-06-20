#include "FlipPlanner.h"
#include "GeometryUtils.h"
#include <cmath>
#include <limits>

std::vector<FlipShot> evaluateFlipShots(
    const std::vector<double>& cueball_pos,
    const std::vector<std::vector<double>>& candidates,
    const std::vector<std::vector<double>>& obstacles,
    const std::vector<std::vector<double>>& walls,
    double bound_radius
) {
    std::vector<FlipShot> flips;
    for (const auto& wall : walls) {
        for (const auto& target : candidates) {
            // Wall reflection logic to compute mirrored target
            double mirror_x = 2 * wall[0] - target[0];
            double mirror_y = 2 * wall[1] - target[1];

            double vec1_x = mirror_x - cueball_pos[0];
            double vec1_y = mirror_y - cueball_pos[1];
            double norm1 = mag(vec1_x, vec1_y);
            if (norm1 == 0) continue;

            double unit1_x = vec1_x / norm1;
            double unit1_y = vec1_y / norm1;

            double contact_x = cueball_pos[0] + unit1_x * (norm1 / 2);
            double contact_y = cueball_pos[1] + unit1_y * (norm1 / 2);

            // Check pre- and post-collision paths
            bool blocked = false;
            for (const auto& obs : obstacles) {
                if (mag(obs[0] - cueball_pos[0], obs[1] - cueball_pos[1]) < 1e-5) continue;
                if (std::abs(dis(unit1_x, unit1_y, cueball_pos[0], cueball_pos[1], obs[0], obs[1])) < bound_radius) {
                    blocked = true;
                    break;
                }
                if (std::abs(dis(target[0] - contact_x, target[1] - contact_y, contact_x, contact_y, obs[0], obs[1])) < bound_radius) {
                    blocked = true;
                    break;
                }
            }

            if (!blocked) {
                FlipShot fs;
                fs.cue_to_wall_vector = {unit1_x * norm1 / 2, unit1_y * norm1 / 2};
                fs.wall_contact_point = {contact_x, contact_y};
                fs.wall_to_target_vector = {target[0] - contact_x, target[1] - contact_y};
                fs.target_coords = target;
                fs.hole_coords = {0, 0};  // Placeholder
                fs.total_distance = mag(fs.cue_to_wall_vector[0], fs.cue_to_wall_vector[1]) +
                                    mag(fs.wall_to_target_vector[0], fs.wall_to_target_vector[1]);
                flips.push_back(fs);
            }
        }
    }
    return flips;
}
