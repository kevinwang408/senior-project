// RobotController.cpp
// ===========================================================================
// Implements robot movement and cue strike control using HRSDK API.
//
// The robot supports point-to-point (PTP), linear (LIN) motion, and digital
// I/O control for strike execution.
// ===========================================================================

#include "RobotController.h"
#include <windows.h>

// ---------------------------------------------------------------------------
// Polls the motion state of the robot arm until it completes current command.
// Required to ensure sequence safety.
// ---------------------------------------------------------------------------
void wait(HROBOT device_id) {
    while (true) {
        if (get_motion_state(device_id) == 1) break; // 1 = Motion complete
    }
}

void moveToPose(HROBOT device_id, const std::vector<double>& pose) {
    double pos[6] = {0};

    // Copy pose vector to robot PTP position array
    for (int i = 0; i < 6; i++) pos[i] = pose[i];

    // Move robot using point-to-point motion (typically top-down)
    ptp_pos(device_id, 0, pos);
    wait(device_id);

    // Lower robot to final strike position using linear motion
    pos[2] = -208; // Hardcoded final z-height
    lin_pos(device_id, 0, 0, pos);
    wait(device_id);
}

void executeStrike(HROBOT device_id) {
    // Use digital output 16 to activate solenoid/striker
    set_digital_output(device_id, 16, false); // Trigger ON
    Sleep(500);                               // Wait 0.5 sec
    set_digital_output(device_id, 16, true);  // Reset
    Sleep(500);                               // Wait
    set_digital_output(device_id, 16, false); // Final off
    wait(device_id);                          // Wait for any motion
}

void returnToHome(HROBOT device_id, const std::vector<double>& home_pose) {
    double home[6] = {0};

    // Copy pose to array
    for (int i = 0; i < 6; i++) home[i] = home_pose[i];

    // Send robot back to home using joint angles
    ptp_axis(device_id, 0, home);
    wait(device_id);
}
