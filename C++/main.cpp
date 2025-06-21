// main.cpp
// ===========================================================================
// Main entry point for robotic billiards system.
// This program loads position data, evaluates shot opportunities, and commands
// a robotic arm to execute a cue strike using either direct or flip shots.
//
// Flow Summary:
// 1. Read CSV inputs (ball positions, wall positions, hole positions)
// 2. Determine valid direct child ball-to-hole shots (using ShotPlanner)
// 3. If none are available, use wall bounce logic (FlipPlanner)
// 4. Select best shot by shortest distance
// 5. Command robot to strike
// ===========================================================================

#include <iostream>
#include "FileIOUtils.h"
#include "ShotPlanner.h"
#include "FlipPlanner.h"
#include "RobotController.h"
#include "GeometryUtils.h"
#include "HRSDK.h"
#include "limits"
void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {};

int main() {
    // Connect to robot controller (assumes HRSDK environment setup)
    
    HROBOT device_id = open_connection("169.254.148.16", 1, callBack);
    if (device_id < 0) {
        std::cerr << "Failed to connect to robot controller." << std::endl;
        return -1;
    }

    // Load all required input data from CSV
    std::vector<std::vector<double>> cueball = loadCSV2D("csv/cueball.csv", 2);   // cueball[0] = mother ball
    std::vector<std::vector<double>> childballs = loadCSV2D("csv/childball.csv", 2);
    std::vector<std::vector<double>> holes = loadCSV2D("csv/holes.csv", 2);
    std::vector<std::vector<double>> walls = loadCSV2D("csv/walls.csv", 2);
    int ball_count = loadSingleInt("csv/ballcount.csv");

    // Generate all possible direct shots
    auto valid_shots = selectClearShots(cueball, holes, childballs, 15);

    std::vector<double> target_ball;
    std::vector<double> target_hole;

    // Select best direct shot (by shortest distance)
    if (!valid_shots.empty()) {
        double min_dist = std::numeric_limits<double>::max();
        for (const auto& shot : valid_shots) {
            double dx = shot.first[0] - shot.second[0];
            double dy = shot.first[1] - shot.second[1];
            double dist = mag(dx, dy);
            if (dist < min_dist) {
                min_dist = dist;
                target_ball = shot.first;
                target_hole = shot.second;
            }
        }
        std::cout << "Selected direct shot.";
    } else {
        // If no direct shot is valid, try flip shots (bank shots)
        auto flip_shots = evaluateFlipShots(cueball[0], childballs, childballs, walls, 15);

        if (!flip_shots.empty()) {
            FlipShot best = flip_shots[0];
            for (const auto& fs : flip_shots) {
                if (fs.total_distance < best.total_distance) {
                    best = fs;
                }
            }
            target_ball = best.target_coords;
            std::cout << "Selected flip shot via wall.";
        } else {
            std::cerr << "No available shots (direct or flip).";
            return -1;
        }
    }

    // Define a target robot pose manually or via mapping (hardcoded here)
    std::vector<double> pose = {350.0, 0.0, -130.0, 180.0, 0.0, 90.0};
    moveToPose(device_id, pose);      // Move to position
    executeStrike(device_id);         // Strike the ball
    returnToHome(device_id, {0, 0, 0, 0, 0, 0});  // Reset to home pose

    disconnect(device_id); // Disconnect from robot
    return 0;
}
