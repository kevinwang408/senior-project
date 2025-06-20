#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include <vector>
#include "HRSDK.h"

// Move robot arm to specified pose
void moveToPose(HROBOT device_id, const std::vector<double>& pose);

// Execute striking sequence
void executeStrike(HROBOT device_id);

// Reset robot arm to home position
void returnToHome(HROBOT device_id, const std::vector<double>& home_pose);

#endif // ROBOT_CONTROLLER_H
