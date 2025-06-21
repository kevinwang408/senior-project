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
        if ((obs_x==x2 && obs_y==y2) || (obs_x==x1 && obs_y==y1)) {
            continue;   
        }
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
    double bound_radius
) {
    //check if there is an obstacle between childball and holes
    std::vector<std::pair<std::vector<double>, std::vector<double>>> child_hole_result;
    //check if there is an obstacle between cueball and childball
    std::vector<std::pair<std::vector<double>, std::vector<double>>> cue_child_result;
    std::vector<std::pair<std::vector<double>, std::vector<double>>> result;
    // For every childball, check all hole paths
    for (const auto& child : childballs) {
        for (const auto& hole : holes) {
            if (!isPathObstructed(child[0], child[1], hole[0], hole[1], childballs, bound_radius)) {
                child_hole_result.emplace_back(child, hole);  // Add valid shot
            }
        }
    }
     for (const auto& child : childballs) {
        for (const auto& cue : cueballs) {
            if (!isPathObstructed(child[0], child[1], cue[0], cue[1], childballs, bound_radius)) {
                
                cue_child_result.emplace_back(cue, child);  // Add valid shot
            }
        }
    }

    for (const auto& child_hole : child_hole_result) {
        const std::vector<double>& child_ball = child_hole.first;   
        const std::vector<double>& hole_coord = child_hole.second;  

        for (const auto& cue_child : cue_child_result) {
            const std::vector<double>& cue_ball = cue_child.first;  

            // 比較兩個子球座標是否相同
            if (child_ball.size() == cue_ball.size()) {
                bool is_same = true;
                for (int i = 0; i < child_ball.size(); ++i) {
                    if (std::abs(child_ball[i] - cue_ball[i]) > 1e-9) {  // 考慮浮點數精度
                        is_same = false;
                        break;
                    }
                }

                // 如果找到相同的子球座標，將其與對應的洞口座標存入 result
                if (is_same) {
                    result.emplace_back(child_ball, hole_coord);
                    break;  // 找到匹配後跳出內層迴圈，處理下一個 child_hole
                }
            }
        }
    }
    
    return result;
}
