#include <iostream>
#include "FileIOUtils.h"
#include "ShotPlanner.h"
#include "FlipPlanner.h"
#include "RobotController.h"
#include "GeometryUtils.h"
#include "HRSDK.h"

void callBack(uint16_t, uint16_t, uint16_t*, int) {}

int main() {
    HROBOT device_id = open_connection("169.254.148.16", 1, callBack);
    if (device_id < 0) {
        std::cout << "Connect failure." << std::endl;
        return -1;
    }

    set_digital_output(device_id, 15, true);
    set_operation_mode(device_id, 1);
    set_override_ratio(device_id, 60);
    set_acc_dec_ratio(device_id, 45);

    std::vector<std::vector<double>> cueball = loadCSV2D("mother_Wcoor.csv", 2);
    std::vector<std::vector<double>> childballs = loadCSV2D("son_Wcoor.csv", 2);
    std::vector<std::vector<double>> holes = {
        {455, -101}, {455, 214}, {152, -101}, {475, 204}, {152, 204}, {-163, 194}
    };

    int ball_count = loadSingleInt("childball_num.csv");

    auto clear_shots = selectClearShots(childballs, holes, childballs, 25.8);
    if (clear_shots.empty()) {
        std::cout << "No valid direct shots, consider flip." << std::endl;
    }

    // Assume choosing the shortest clear shot
    std::vector<double> final_target, final_hole;
    double min_dist = 1e9;
    for (const auto& shot : clear_shots) {
        double dist = mag(shot.first[0] - cueball[0][0], shot.first[1] - cueball[0][1]) +
                      mag(shot.first[0] - shot.second[0], shot.first[1] - shot.second[1]);
        if (dist < min_dist) {
            min_dist = dist;
            final_target = shot.first;
            final_hole = shot.second;
        }
    }

    double vector_x = final_target[0] - final_hole[0];
    double vector_y = final_target[1] - final_hole[1];
    double magnitude = mag(vector_x, vector_y);
    vector_x /= magnitude;
    vector_y /= magnitude;

    double impact_x = final_target[0] - (25.6 - 3) * vector_x;
    double impact_y = final_target[1] - (25.6 - 3) * vector_y;

    std::vector<double> pose = {
        impact_x, impact_y, -200, 0, 0, -90  // Assuming no rotation offset
    };

    moveToPose(device_id, pose);
    executeStrike(device_id);
    std::vector<double> home_pose = {90, 0, 15, 0, -105, 0};
    returnToHome(device_id, home_pose);

    disconnect(device_id);
    return 0;
}
