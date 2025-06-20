#include "RobotController.h"
#include <windows.h>

void wait(HROBOT device_id) {
    while (true) {
        if (get_motion_state(device_id) == 1) {
            break;
        }
    }
}

void moveToPose(HROBOT device_id, const std::vector<double>& pose) {
    double pos[6] = {0};
    for (int i = 0; i < 6; i++) pos[i] = pose[i];

    ptp_pos(device_id, 0, pos);  // Move to pose
    wait(device_id);

    pos[2] = -208;               // Lower to hit position
    lin_pos(device_id, 0, 0, pos);
    wait(device_id);
}

void executeStrike(HROBOT device_id) {
    set_digital_output(device_id, 16, false);
    Sleep(500);
    set_digital_output(device_id, 16, true);
    Sleep(500);
    set_digital_output(device_id, 16, false);
    wait(device_id);
}

void returnToHome(HROBOT device_id, const std::vector<double>& home_pose) {
    double home[6] = {0};
    for (int i = 0; i < 6; i++) home[i] = home_pose[i];
    ptp_axis(device_id, 0, home);
    wait(device_id);
}
