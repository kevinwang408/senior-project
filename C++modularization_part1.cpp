// BilliardGameSystem.h - Header file with includes and constants
#include <Python.h>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <cstdio>
#include <math.h>
#include "HRSDK.h"

using namespace std;

// ============================================================================
// CONSTANTS AND CONFIGURATION
// ============================================================================

// Ball and table dimensions
#define BALL_DIAMETER 25.6
#define BOUND_VAL2 25.8139  // 2 * Radius

// Position correction offsets for arm hitting point
#define Y_POS_CORRECT -15   // Arm hitting point Y correction
#define X_POS_CORRECT 5     // Arm hitting point X correction  
#define Z_CORRECT -8        // Z axis correction
#define Z_OFFSET -100       // Z position offset
#define CALIBRATION -3      // Calibration offset for gripper mounting

// Table boundary positions (in world coordinates)
#define TOP_BOUNDARY 455
#define MIDDLE_BOUNDARY 152
#define BOTTOM_BOUNDARY -168
#define LEFT_BOUNDARY -101
#define RIGHT_BOUNDARY 204

// Build configuration
#define x64
#define HITTER_A 0

// ============================================================================
// LIBRARY LINKING
// ============================================================================

#ifdef x64
#pragma comment(lib,"C:/Users/wang8/source/repos/ConsoleApplication2/ConsoleApplication2/HRSDK.lib")
#pragma comment(lib,"C:/Users/wang8/source/repos/cflip/cflip/opencv_world470d.lib")
#else
#pragma comment(lib, "../../../../lib/x86/HRSDK.lib")
#endif

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Mathematical utility functions
double INNER_PRODUCT(double x1, double y1, double x2, double y2);
double mag(double x, double y);  // Calculate magnitude of 2D vector
double COS_VAL(double x1, double y1, double x2, double y2);
double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0);

// Robot control functions
void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int);
void wait(HROBOT device_id);
void to_enter_point(HROBOT device_id);

// ============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// ============================================================================

/**
 * Callback function for robot communication (required by HRSDK)
 * Currently empty implementation
 */
void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {
    // Empty callback - required by HRSDK but not used
}

/**
 * Wait for robot motion to complete
 * @param device_id: Robot device identifier
 */
void wait(HROBOT device_id) {
    while (true) {
        if (get_motion_state(device_id) == 1) {
            break;  // Motion completed
        }
    }
}

/**
 * Calculate 2D vector magnitude
 * @param x: X component
 * @param y: Y component  
 * @return: Vector magnitude
 */
double mag(double x, double y) {
    return sqrt(x * x + y * y);
}

// ============================================================================
// MAIN GAME LOGIC CLASS
// ============================================================================

class BilliardGameSystem {
private:
    vector<vector<double>> world_childball_coor;  // Child ball world coordinates
    vector<double> world_cueballcoor;             // Cue ball world coordinates
    vector<vector<double>> world_hole_coor;       // Hole positions
    vector<double> target_ball_coor;              // Selected target ball
    
public:
    BilliardGameSystem();
    bool loadBallPositions();
    void initializeHolePositions();
    void findOptimalTarget();
    void calculateTargetBallHoleVectors();
    void detectObstructions();
};

/**
 * Constructor - Initialize hole positions
 */
BilliardGameSystem::BilliardGameSystem() {
    world_cueballcoor.resize(2, 0);
    target_ball_coor.resize(3, 0);
    target_ball_coor[2] = 1000;  // Initialize with large distance value
    initializeHolePositions();
}

/**
 * Initialize standard billiard table hole positions
 */
void BilliardGameSystem::initializeHolePositions() {
    world_hole_coor.resize(6, vector<double>(2, 0));
    world_hole_coor = {
        {BOTTOM_BOUNDARY, LEFT_BOUNDARY},       // Bottom-left hole
        {TOP_BOUNDARY, LEFT_BOUNDARY + 10},     // Top-left hole  
        {MIDDLE_BOUNDARY, LEFT_BOUNDARY},       // Middle-left hole
        {TOP_BOUNDARY + 20, RIGHT_BOUNDARY},    // Top-right hole
        {MIDDLE_BOUNDARY, RIGHT_BOUNDARY},      // Middle-right hole
        {BOTTOM_BOUNDARY + 5, RIGHT_BOUNDARY - 10}  // Bottom-right hole
    };
}

/**
 * Load ball positions from CSV files
 * @return: true if successful, false otherwise
 */
bool BilliardGameSystem::loadBallPositions() {
    // File streams for reading ball position data
    fstream son_Wcoor, childball_num, mother_Wcoor;
    
    // Open CSV files containing ball position data
    son_Wcoor.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/son_Wcoor.csv", ios::in);
    childball_num.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/childball_num.csv", ios::in);
    mother_Wcoor.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/mother_Wcoor.csv", ios::in);
    
    if (!son_Wcoor.is_open() || !childball_num.is_open() || !mother_Wcoor.is_open()) {
        return false;  // File opening failed
    }
    
    char buffer[1024] = {0};
    int n = 0;  // Number of child balls
    
    // Read number of child balls
    while (childball_num.getline(buffer, 1024)) {
        sscanf_s(buffer, "%d", &n);
    }
    
    // Initialize child ball coordinate array
    world_childball_coor.resize(n, vector<double>(2, 0));
    
    // Read child ball world coordinates
    int i = 0;
    while (son_Wcoor.getline(buffer, 1024)) {
        sscanf_s(buffer, "%lf,%lf", &world_childball_coor[i][0], &world_childball_coor[i][1]);
        i++;
    }
    
    // Read cue ball (mother ball) world coordinates
    while (mother_Wcoor.getline(buffer, 1024)) {
        sscanf_s(buffer, "%lf,%lf", &world_cueballcoor[0], &world_cueballcoor[1]);
    }
    
    // Close file streams
    son_Wcoor.close();
    childball_num.close();
    mother_Wcoor.close();
    
    // Debug output
    cout << "Cue ball coordinates: " << world_cueballcoor[0] << " " << world_cueballcoor[1] << endl;
    
    return true;
}

/**
 * Find optimal target ball based on distance to nearest hole
 * Strategy: Select the ball closest to any hole
 */
void BilliardGameSystem::findOptimalTarget() {
    int n = world_childball_coor.size();
    vector<vector<double>> test_vector(6 * n, vector<double>(5, 0));
    int test_index = 0;
    
    // Test all ball-hole combinations to find closest
    for (int hole_idx = 0; hole_idx < 6; hole_idx++) {          // All holes
        for (int ball_idx = 0; ball_idx < n; ball_idx++) {      // All child balls
            
            // Calculate distance from ball to hole
            double distance = mag(
                world_hole_coor[hole_idx][0] - world_childball_coor[ball_idx][0],
                world_hole_coor[hole_idx][1] - world_childball_coor[ball_idx][1]
            );
            
            // Store test data: ball_x, ball_y, hole_x, hole_y, distance
            test_vector[test_index][0] = world_childball_coor[ball_idx][0];
            test_vector[test_index][1] = world_childball_coor[ball_idx][1];
            test_vector[test_index][2] = world_hole_coor[hole_idx][0];
            test_vector[test_index][3] = world_hole_coor[hole_idx][1];
            test_vector[test_index][4] = distance;
            test_index++;
            
            // Update target ball if this is the closest found so far
            if (distance < target_ball_coor[2]) {
                target_ball_coor[0] = world_childball_coor[ball_idx][0];
                target_ball_coor[1] = world_childball_coor[ball_idx][1];
                target_ball_coor[2] = distance;
            }
        }
    }
    
    // Save test results to file for analysis
    ofstream test_output("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/test_vector.csv");
    for (int i = 0; i < test_index; i++) {
        test_output << test_vector[i][0] << "," << test_vector[i][1] << "," 
                   << test_vector[i][2] << "," << test_vector[i][3] << "," 
                   << test_vector[i][4] << endl;
    }
    test_output.close();
    
    // Debug output
    cout << "Target ball coordinates: " << target_ball_coor[0] << " " 
         << target_ball_coor[1] << " Distance: " << target_ball_coor[2] << endl;
}

/**
 * Calculate vectors from target ball to all holes
 * Used for trajectory planning and obstruction detection
 */
void BilliardGameSystem::calculateTargetBallHoleVectors() {
    vector<vector<double>> rel_targetball_hole(6, vector<double>(2, 0));
    
    // Calculate relative vectors from target ball to each hole
    for (int i = 0; i < 6; i++) {
        rel_targetball_hole[i][0] = world_hole_coor[i][0] - target_ball_coor[0];
        rel_targetball_hole[i][1] = world_hole_coor[i][1] - target_ball_coor[1];
    }
    
    // Initialize obstruction detection arrays
    vector<vector<double>> inter_detect(6, vector<double>(2, 0));
    vector<vector<double>> inter_vector_detect(6, vector<double>(2, 0));
    
    // Copy hole positions and vectors for obstruction analysis
    for (int i = 0; i < 6; i++) {
        inter_detect[i][0] = world_hole_coor[i][0];
        inter_detect[i][1] = world_hole_coor[i][1];
        inter_vector_detect[i][0] = rel_targetball_hole[i][0];
        inter_vector_detect[i][1] = rel_targetball_hole[i][1];
    }
}

// ============================================================================
// TRAJECTORY ANALYSIS AND SHOT PLANNING
// ============================================================================

/**
 * Calculate point-to-line distance for obstruction detection
 * @param vec_x, vec_y: Direction vector components
 * @param pass_x, pass_y: Point on the line
 * @param x0, y0: Point to calculate distance from
 * @return: Perpendicular distance from point to line
 */
double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0) {
    // Using cross product to calculate perpendicular distance
    return abs(vec_x * (y0 - pass_y) - vec_y * (x0 - pass_x)) / mag(vec_x, vec_y);
}

/**
 * Calculate cosine value for angle between two vectors
 * @param x1, y1: First vector components
 * @param x2, y2: Second vector components
 * @return: Cosine of angle between vectors
 */
double COS_VAL(double x1, double y1, double x2, double y2) {
    return (x1 * x2 + y1 * y2) / (mag(x1, y1) * mag(x2, y2));
}

class TrajectoryAnalyzer {
private:
    vector<vector<double>> available_holes;
    vector<vector<double>> available_hole_vectors;
    vector<vector<double>> collision_points;
    vector<vector<double>> direct_shots;
    vector<vector<double>> bank_shots;
    
public:
    void detectObstructions(const vector<vector<double>>& world_childball_coor,
                           const vector<vector<double>>& world_hole_coor,
                           const vector<double>& target_ball_coor,
                           const vector<vector<double>>& rel_targetball_hole);
    
    void calculateCollisionPoints(const vector<double>& target_ball_coor);
    void analyzeDirectShots(const vector<vector<double>>& world_childball_coor,
                           const vector<double>& world_cueballcoor,
                           const vector<double>& target_ball_coor);
    void calculateBankShots(const vector<double>& world_cueballcoor);
    void saveAnalysisResults();
    
    int getAvailableHoleCount() const { return available_holes.size(); }
    const vector<vector<double>>& getDirectShots() const { return direct_shots; }
    const vector<vector<double>>& getBankShots() const { return bank_shots; }
};

/**
 * Detect obstructions between target ball and holes
 * Marks holes as unavailable if other balls block the path
 */
void TrajectoryAnalyzer::detectObstructions(const vector<vector<double>>& world_childball_coor,
                                          const vector<vector<double>>& world_hole_coor,
                                          const vector<double>& target_ball_coor,
                                          const vector<vector<double>>& rel_targetball_hole) {
    
    vector<vector<double>> targetball_hole_interrupted(100, vector<double>(2, 0));
    vector<vector<double>> inter_detect(6, vector<double>(2, 0));
    vector<vector<double>> inter_vector_detect(6, vector<double>(2, 0));
    
    // Initialize detection arrays with original hole data
    for (int i = 0; i < 6; i++) {
        inter_detect[i][0] = world_hole_coor[i][0];
        inter_detect[i][1] = world_hole_coor[i][1];
        inter_vector_detect[i][0] = rel_targetball_hole[i][0];
        inter_vector_detect[i][1] = rel_targetball_hole[i][1];
    }
    
    int obstruction_idx = 0;
    int n = world_childball_coor.size();
    
    // Check each hole-target ball path for obstructions
    for (int hole_idx = 0; hole_idx < 6; hole_idx++) {
        for (int ball_idx = 0; ball_idx < n; ball_idx++) {
            
            // Calculate perpendicular distance from ball to target-hole line
            double perpendicular_distance = dis(
                world_hole_coor[hole_idx][0] - target_ball_coor[0],
                world_hole_coor[hole_idx][1] - target_ball_coor[1],
                target_ball_coor[0], target_ball_coor[1],
                world_childball_coor[ball_idx][0], world_childball_coor[ball_idx][1]
            );
            
            // Check if ball interferes with path (within 2 ball radii)
            if (abs(perpendicular_distance) < BOUND_VAL2) {
                
                double target_to_hole_dist = mag(rel_targetball_hole[hole_idx][0], rel_targetball_hole[hole_idx][1]);
                double interfering_ball_to_hole_dist = mag(
                    world_childball_coor[ball_idx][0] - world_hole_coor[hole_idx][0],
                    world_childball_coor[ball_idx][1] - world_hole_coor[hole_idx][1]
                );
                
                // If interfering ball is closer to hole, mark hole as blocked
                if (interfering_ball_to_hole_dist < target_to_hole_dist) {
                    targetball_hole_interrupted[obstruction_idx][0] = world_hole_coor[hole_idx][0];
                    targetball_hole_interrupted[obstruction_idx][1] = world_hole_coor[hole_idx][1];
                    
                    // Mark hole as unavailable (using large number as flag)
                    inter_detect[hole_idx][0] = 10000;
                    inter_detect[hole_idx][1] = 10000;
                    inter_vector_detect[hole_idx][0] = 10000;
                    inter_vector_detect[hole_idx][1] = 10000;
                    
                    cout << "Blocked hole coordinates: " << targetball_hole_interrupted[obstruction_idx][0] 
                         << " " << targetball_hole_interrupted[obstruction_idx][1] << endl;
                    cout << "Interfering ball coordinates: " << world_childball_coor[ball_idx][0] 
                         << " " << world_childball_coor[ball_idx][1] << endl;
                    
                    obstruction_idx++;
                    break;
                }
            }
        }
    }
    
    // Extract available (unblocked) holes
    available_holes.clear();
    available_hole_vectors.clear();
    int available_count = 0;
    
    cout << "Available hole coordinates and vectors to target ball:" << endl;
    for (int i = 0; i < 6; i++) {
        if (inter_detect[i][0] < 9000) {  // Not marked as blocked
            available_holes.push_back({inter_detect[i][0], inter_detect[i][1]});
            available_hole_vectors.push_back({inter_vector_detect[i][0], inter_vector_detect[i][1]});
            
            cout << inter_detect[i][0] << " " << inter_detect[i][1] << endl;
            cout << inter_vector_detect[i][0] << " " << inter_vector_detect[i][1] << endl;
            available_count++;
        }
    }
    
    cout << "Number of available holes: " << available_count << endl;
}

/**
 * Calculate collision points for each available hole
 * These are points where cue ball should hit target ball
 */
void TrajectoryAnalyzer::calculateCollisionPoints(const vector<double>& target_ball_coor) {
    collision_points.clear();
    vector<vector<double>> unit_hole_vectors;
    
    // Convert hole vectors to unit vectors
    for (size_t i = 0; i < available_hole_vectors.size(); i++) {
        double vector_magnitude = mag(available_hole_vectors[i][0], available_hole_vectors[i][1]);
        unit_hole_vectors.push_back({
            available_hole_vectors[i][0] / vector_magnitude,
            available_hole_vectors[i][1] / vector_magnitude
        });
    }
    
    // Calculate collision points (offset from target ball center)
    cout << "Collision point coordinates:" << endl;
    for (size_t i = 0; i < unit_hole_vectors.size(); i++) {
        double collision_x = target_ball_coor[0] - (BALL_DIAMETER - 5) * unit_hole_vectors[i][0];
        double collision_y = target_ball_coor[1] - (BALL_DIAMETER - 5) * unit_hole_vectors[i][1];
        
        collision_points.push_back({collision_x, collision_y});
        cout << collision_x << " " << collision_y << endl;
    }
}

/**
 * Analyze direct shot feasibility for each collision point
 * Checks for obstructions and angle constraints
 */
void TrajectoryAnalyzer::analyzeDirectShots(const vector<vector<double>>& world_childball_coor,
                                          const vector<double>& world_cueballcoor,
                                          const vector<double>& target_ball_coor) {
    
    direct_shots.clear();
    int collision_count = collision_points.size();
    int n = world_childball_coor.size();
    
    cout << "Direct shot vectors and feasibility:" << endl;
    
    for (int shot_idx = 0; shot_idx < collision_count; shot_idx++) {
        // Initialize shot data: [collision_x, collision_y, cue_x, cue_y, vector_x, vector_y, feasibility_flag]
        vector<double> shot_data(7, 0);
        
        shot_data[0] = collision_points[shot_idx][0];  // Collision point X
        shot_data[1] = collision_points[shot_idx][1];  // Collision point Y
        shot_data[2] = world_cueballcoor[0];           // Cue ball X
        shot_data[3] = world_cueballcoor[1];           // Cue ball Y
        shot_data[4] = collision_points[shot_idx][0] - world_cueballcoor[0];  // Shot vector X
        shot_data[5] = collision_points[shot_idx][1] - world_cueballcoor[1];  // Shot vector Y
        shot_data[6] = 0;  // Feasibility flag (0 = feasible, 10000 = blocked)
        
        // Check for obstructions along cue ball to collision point path
        for (int ball_idx = 0; ball_idx < n; ball_idx++) {
            double perpendicular_distance = dis(
                shot_data[4], shot_data[5],  // Shot vector
                world_cueballcoor[0], world_cueballcoor[1],  // Cue ball position
                world_childball_coor[ball_idx][0], world_childball_coor[ball_idx][1]  // Test ball position
            );
            
            if (abs(perpendicular_distance) < BOUND_VAL2) {  // Ball interferes with shot path
                double cue_to_collision_dist = mag(shot_data[4], shot_data[5]);
                double cue_to_interfering_ball_dist = mag(
                    world_childball_coor[ball_idx][0] - shot_data[0],
                    world_childball_coor[ball_idx][1] - shot_data[1]
                );
                double interfering_ball_to_target_dist = mag(
                    world_childball_coor[ball_idx][0] - target_ball_coor[0],
                    world_childball_coor[ball_idx][1] - target_ball_coor[1]
                );
                
                // Ignore if interfering ball is actually the target ball itself
                if (interfering_ball_to_target_dist > 20) {
                    if (cue_to_interfering_ball_dist < cue_to_collision_dist) {
                        shot_data[6] = 10000;  // Mark as blocked
                        cout << "Path obstruction - Shot " << (shot_idx + 1) << " blocked" << endl;
                        break;
                    }
                }
            }
        }
        
        // Check shot angle constraint (avoid extreme angles)
        if (shot_data[6] < 9000) {  // If not already blocked
            double shot_angle = acos(COS_VAL(
                shot_data[4], shot_data[5],  // Cue to collision vector
                target_ball_coor[0] - shot_data[0], target_ball_coor[1] - shot_data[1]  // Collision to target vector
            )) * 180 / M_PI;
            
            if (shot_angle > 70) {  // Angle too steep
                shot_data[6] = 10000;
                cout << "Angle constraint - Shot " << (shot_idx + 1) << " blocked" << endl;
            }
        }
        
        direct_shots.push_back(shot_data);
        cout << shot_data[0] << " " << shot_data[1] << " " << shot_data[2] << endl;
    }
    
    cout << "Direct shot count: " << collision_count << endl;
}

/**
 * Calculate bank shot trajectories using table wall reflections
 * Generates bank shots off each wall for each collision point
 */
void TrajectoryAnalyzer::calculateBankShots(const vector<double>& world_cueballcoor) {
    bank_shots.clear();
    int collision_count = collision_points.size();
    int bank_shot_count = 0;
    
    // For each collision point, calculate bank shots off each wall
    for (int i = 0; i < collision_count; i++) {
        double collision_x = collision_points[i][0];
        double collision_y = collision_points[i][1];
        
        // Bank shot off top wall
        double rel_top = TOP_BOUNDARY - collision_x - 13;  // Account for ball radius
        double cue_rel_top = TOP_BOUNDARY - world_cueballcoor[0] - 13;
        vector<double> top_bank_shot(8, 0);
        top_bank_shot[0] = collision_x + rel_top + cue_rel_top;  // Bank point X
        top_bank_shot[1] = collision_y;                          // Bank point Y
        top_bank_shot[2] = top_bank_shot[0] - world_cueballcoor[0];  // Cue to bank vector X
        top_bank_shot[3] = top_bank_shot[1] - world_cueballcoor[1];  // Cue to bank vector Y
        double bank_ratio = cue_rel_top / top_bank_shot[2];
        top_bank_shot[4] = top_bank_shot[2] * bank_ratio;  // Reflected vector X
        top_bank_shot[5] = top_bank_shot[3] * bank_ratio;  // Reflected vector Y
        top_bank_shot[6] = collision_x;  // Final collision point X
        top_bank_shot[7] = collision_y;  // Final collision point Y
        bank_shots.push_back(top_bank_shot);
        
        // Bank shot off left wall
        double rel_left = LEFT_BOUNDARY - collision_y + 13;
        double cue_rel_left = LEFT_BOUNDARY - world_cueballcoor[1] + 13;
        vector<double> left_bank_shot(8, 0);
        left_bank_shot[0] = collision_x;
        left_bank_shot[1] = collision_y + rel_left + cue_rel_left;
        left_bank_shot[2] = left_bank_shot[0] - world_cueballcoor[0];
        left_bank_shot[3] = left_bank_shot[1] - world_cueballcoor[1];
        bank_ratio = cue_rel_left / left_bank_shot[3];
        left_bank_shot[4] = left_bank_shot[2] * bank_ratio;
        left_bank_shot[5] = left_bank_shot[3] * bank_ratio;
        left_bank_shot[6] = collision_x;
        left_bank_shot[7] = collision_y;
        bank_shots.push_back(left_bank_shot);
        
        // Bank shot off bottom wall
        double rel_bottom = BOTTOM_BOUNDARY - collision_x + 13;
        double cue_rel_bottom = BOTTOM_BOUNDARY - world_cueballcoor[0] + 13;
        vector<double> bottom_bank_shot(8, 0);
        bottom_bank_shot[0] = collision_x + rel_bottom + cue_rel_bottom;
        bottom_bank_shot[1] = collision_y;
        bottom_bank_shot[2] = bottom_bank_shot[0] - world_cueballcoor[0];
        bottom_bank_shot[3] = bottom_bank_shot[1] - world_cueballcoor[1];
        bank_ratio = cue_rel_bottom / bottom_bank_shot[2];
        bottom_bank_shot[4] = bottom_bank_shot[2] * bank_ratio;
        bottom_bank_shot[5] = bottom_bank_shot[3] * bank_ratio;
        bottom_bank_shot[6] = collision_x;
        bottom_bank_shot[7] = collision_y;
        bank_shots.push_back(bottom_bank_shot);
        
        // Bank shot off right wall
        double rel_right = RIGHT_BOUNDARY - collision_y - 13;
        double cue_rel_right = RIGHT_BOUNDARY - world_cueballcoor[1] - 13;
        vector<double> right_bank_shot(8, 0);
        right_bank_shot[0] = collision_x;
        right_bank_shot[1] = collision_y + rel_right + cue_rel_right;
        right_bank_shot[2] = right_bank_shot[0] - world_cueballcoor[0];
        right_bank_shot[3] = right_bank_shot[1] - world_cueballcoor[1];
        bank_ratio = cue_rel_right / right_bank_shot[3];
        right_bank_shot[4] = right_bank_shot[2] * bank_ratio;
        right_bank_shot[5] = right_bank_shot[3] * bank_ratio;
        right_bank_shot[6] = collision_x;
        right_bank_shot[7] = collision_y;
        bank_shots.push_back(right_bank_shot);
        
        bank_shot_count += 4;  // 4 walls per collision point
    }
    
    cout << "Bank shot count after first reflection: " << bank_shot_count << endl;
}

/**
 * Save analysis results to CSV files for debugging and visualization
 */
void TrajectoryAnalyzer::saveAnalysisResults() {
    // Save direct shot analysis
    ofstream direct_file("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/direct_collide.csv");
    for (const auto& shot : direct_shots) {
        for (size_t i = 0; i < shot.size(); i++) {
            direct_file << shot[i];
            if (i < shot.size() - 1) direct_file << ",";
        }
        direct_file << endl;
    }
    direct_file.close();
}

/**
 * Main entry point function for robot control
 * @param device_id: Robot device identifier
 */
void to_enter_point(HROBOT device_id) {
    BilliardGameSystem game;
    TrajectoryAnalyzer analyzer;
    
    // Load ball positions from files
    if (!game.loadBallPositions()) {
        cout << "Error: Failed to load ball positions!" << endl;
        return;
    }
    
    // Find optimal target ball
    game.findOptimalTarget();
    
    // Calculate trajectory vectors and analyze shots
    game.calculateTargetBallHoleVectors();
    
    // Continue with robot control implementation...
}