// ShotPlanner.cpp
// ===========================================================================
// Implements core shot filtering logic based on geometric interference
// checking with other balls on the table.
// ===========================================================================

#include "ShotPlanner.h"
#include "GeometryUtils.h"
#include <cmath>
#include <limits>

bool isPathObstructed(
    double x1, double y1, double x2, double y2,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
) {
    for (const auto& obs : obstacles) {
        double obs_x = obs[0];
        double obs_y = obs[1];

        // Calculate perpendicular distance to line (x1,y1)->(x2,y2)
        double d = dis(x2 - x1, y2 - y1, x1, y1, obs_x, obs_y);

        // If close enough to line AND within the segment bounds, it's an obstruction
        if (std::abs(d) < bound_radius) {
            double mag_target = mag(x2 - x1, y2 - y1);
            double mag_obs = mag(obs_x - x1, obs_y - y1);
            if (mag_obs < mag_target) return true;
        }
    }
    return false;
}

std::vector<std::pair<std::vector<double>, std::vector<double>>> selectClearShots(
    const std::vector<std::vector<double>>& cueballs,
    const std::vector<std::vector<double>>& holes,
    const std::vector<std::vector<double>>& childballs,
    const std::vector<std::vector<double>>& obstacles,
    double bound_radius
) {
    std::vector<std::pair<std::vector<double>, std::vector<double>>> result;

    // For every cueball, check all hole paths
    for (const auto& cue : cueballs) {
        for (const auto& hole : holes) {
            if (!isPathObstructed(cue[0], cue[1], hole[0], hole[1], obstacles, bound_radius)) {
                result.emplace_back(cue, hole);  // Add valid shot
            }
        }
    }

    return result;
}
