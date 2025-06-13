#include "billiard_trajectory.h"
#include <iostream>
#include <vector>

// 主程式整合類別
class BilliardGameController {
private:
    TrajectoryCalculator trajectory_calc;
    RobotController* robot_controller;
    RobotInitializer robot_init;
    bool is_initialized;
    
    // 遊戲參數
    struct GameConfig {
        string python_script_path = "D:/hiwin/vscode_python/circle_detect_640.py";
        string robot_ip = "192.168.100.45";
        double z_height = 15.0;
        double z_correct_height = 10.0;
        double hitter_offset = 5.0;
        
        // CSV輸出路徑
        string base_path = "C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/";
        string flip_point_file = base_path + "flip_once_point.csv";
        string flip_vector_file = base_path + "flip_once_vector.csv";
        string final_coord_file = base_path + "final_coor.csv";
        string visualization_file = base_path + "circle.csv";
    };
    
    GameConfig config;
    
public:
    BilliardGameController() : robot_controller(nullptr), is_initialized(false) {}
    
    ~BilliardGameController() {
        delete robot_controller;
    }
    
    // 初始化系統
    bool initialize() {
        // 1. 執行Python影像辨識
        if (!PythonScriptRunner::runImageDetection(config.python_script_path)) {
            cout << "Python腳本執行失敗" << endl;
            return false;
        }
        
        // 2. 初始化機械手臂連接
        if (!robot_init.initializeConnection(config.robot_ip)) {
            cout << "機械手臂連接失敗" << endl;
            return false;
        }
        
        // 3. 創建機械手臂控制器
        robot_controller = new RobotController(robot_init.getDeviceId());
        is_initialized = true;
        
        return true;
    }
    
    // 主要遊戲執行流程
    void executeGame(const vector<double>& world_cueballcoor,
                    const vector<double>& target_ball_coor,
                    const vector<vector<double>>& world_childball_coor,
                    const vector<vector<double>>& collide_coor_project,
                    const vector<vector<double>>& direct_collide_vector,
                    const vector<vector<double>>& collide_coor,
                    int project_count, int idx, int n,
                    double top, double right, double left, double middle, double bottom) {
        
        if (!is_initialized) {
            cout << "系統未初始化" << endl;
            return;
        }
        
        // 1. 計算彈射軌跡
        auto flip_trajectory = calculateFlipTrajectory(world_cueballcoor, target_ball_coor, 
                                                     world_childball_coor, collide_coor_project,
                                                     collide_coor, project_count, idx, n);
        
        // 2. 尋找最佳擊球點
        auto optimal_target = findOptimalHitTarget(flip_trajectory, direct_collide_vector,
                                                 world_cueballcoor, target_ball_coor, idx);
        
        // 3. 執行力道控制和打擊
        executeHitSequence(optimal_target, world_cueballcoor, target_ball_coor);
        
        // 4. 輸出數據和可視化
        exportGameData(flip_trajectory, optimal_target, world_cueballcoor, target_ball_coor,
                      collide_coor, top, right, left, middle, bottom, project_count, idx);
        
        // 5. 執行機械手臂主控制流程
        robot_init.executeMainControl();
    }
    
private:
    // 計算彈射軌跡
    TrajectoryCalculator::FlipTrajectory calculateFlipTrajectory(
        const vector<double>& world_cueballcoor,
        const vector<double>& target_ball_coor,
        const vector<vector<double>>& world_childball_coor,
        const vector<vector<double>>& collide_coor_project,
        const vector<vector<double>>& collide_coor,
        int project_count, int idx, int n) {
        
        // 初始化彈射軌跡
        auto trajectory = trajectory_calc.initializeFlipTrajectory(project_count);
        
        // 計算彈射點和向量
        trajectory_calc.calculateFlipPointsAndVectors(trajectory, world_cueballcoor, 
                                                    collide_coor_project, project_count);
        
        // 檢測干擾
        trajectory_calc.detectInterference(trajectory, world_cueballcoor, world_childball_coor,
                                         target_ball_coor, project_count, n);
        
        // 檢測角度限制
        trajectory_calc.checkAngleConstraints(trajectory, collide_coor, target_ball_coor, idx);
        
        // 檢測洞口附近
        trajectory_calc.checkHoleProximity(trajectory, idx * 4);
        
        // 標記無效軌跡
        trajectory_calc.markInvalidTrajectories(trajectory, idx * 4);
        
        return trajectory;
    }
    
    // 尋找最佳擊球目標
    TrajectoryCalculator::FinalTarget findOptimalHitTarget(
        const TrajectoryCalculator::FlipTrajectory& flip_trajectory,
        const vector<vector<double>>& direct_collide_vector,
        const vector<double>& world_cueballcoor,
        const vector<double>& target_ball_coor,
        int idx) {
        
        // 先嘗試直接擊打
        auto target = trajectory_calc.findOptimalTarget(flip_trajectory, direct_collide_vector,
                                                      world_cueballcoor, idx, 1);
        
        // 如果找不到直接擊打點，嘗試彈射
        if (!target.found) {
            target = trajectory_calc.findOptimalTarget(flip_trajectory, direct_collide_vector,
                                                     world_cueballcoor, idx, 2);
        }
        
        // 如果還是找不到，使用備用方案
        if (!target.found) {
            target.coordinate = robot_controller->calculateFallbackTarget(world_cueballcoor, target_ball_coor);
            target.found = true;
            
            // 計算備用方案的單位向量
            vector<double> direction(2);
            direction[0] = target_ball_coor[0] - world_cueballcoor[0];
            direction[1] = target_ball_coor[1] - world_cueballcoor[1];
            double mag = sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
            target.unit_vector[0] = direction[0] / mag;
            target.unit_vector[1] = direction[1] / mag;
            target.distance = mag;
        }
        
        return target;
    }
    
    // 執行打擊序列
    void executeHitSequence(const TrajectoryCalculator::FinalTarget& target,
                          const vector<double>& world_cueballcoor,
                          const vector<double>& target_ball_coor) {
        
        cout << "力道控制前: " << target.coordinate[0] << " " << target.coordinate[1] << endl;
        
        // 計算打擊角度
        double theta = robot_controller->calculateHitAngle(target.unit_vector);
        
        // 設定力道控制
        auto power_control = robot_controller->setPowerControl(target.distance, 
                                                             target_ball_coor.size() > 3 ? target_ball_coor[3] : 0);
        robot_controller->applyPowerSettings(power_control);
        
        // 限制座標在球檯範圍內
        auto constrained_coord = robot_controller->constrainToTableBoundary(target.coordinate, 
                                                                           config.hitter_offset);
        
        cout << "力道控制後: " << constrained_coord[0] << " " << constrained_coord[1] << endl;
        
        // 執行打擊
        robot_controller->executeHit(constrained_coord, target.unit_vector, theta, 
                                   config.z_height, config.z_correct_height);
        
        // 輸出最終信息
        cout << "母球座標: " << world_cueballcoor[0] << ", " << world_cueballcoor[1] << endl;
        cout << "目標球座標: " << target_ball_coor[0] << ", " << target_ball_coor[1] << endl;
        cout << "最終擊球點: " << constrained_coord[0] << ", " << constrained_coord[1] << endl;
    }
    
    // 輸出遊戲數據
    void exportGameData(const TrajectoryCalculator::FlipTrajectory& trajectory,
                       const TrajectoryCalculator::FinalTarget& target,
                       const vector<double>& world_cueballcoor,
                       const vector<double>& target_ball_coor,
                       const vector<vector<double>>& collide_coor,
                       double top, double right, double left, double middle, double bottom,
                       int project_count, int idx) {
        
        // 輸出軌跡數據
        trajectory_calc.exportTrajectoryData(trajectory, config.flip_point_file, 
                                           config.flip_vector_file, project_count);
        
        // 輸出最終座標
        DataExporter::exportFinalCoordinate(target.coordinate, config.final_coord_file);
        
        // 輸出可視化數據
        DataExporter::exportVisualizationData(config.visualization_file, top, right, left, 
                                            middle, bottom, world_cueballcoor, target_ball_coor,
                                            collide_coor, idx);
    }
};

// 主函數範例
int main() {
    BilliardGameController game_controller;
    
    // 初始化系統
    if (!game_controller.initialize()) {
        cout << "系統初始化失敗" << endl;
        return -1;
    }
    
    // 這裡應該從影像辨識或其他來源獲取實際數據
    // 以下為示例數據，實際使用時需要替換為真實數據
    vector<double> world_cueballcoor = {100, 50};
    vector<double> target_ball_coor = {200, 100, 0, 150}; // x, y, ?, distance_to_hole
    vector<vector<double>> world_childball_coor; // 其他子球座標
    vector<vector<double>> collide_coor_project; // 撞擊座標投影
    vector<vector<double>> direct_collide_vector; // 直接撞擊向量
    vector<vector<double>> collide_coor; // 撞擊座標
    
    // 球檯參數
    double top = 200, right = 500, left = -200, middle = 50, bottom = -100;
    int project_count = 12, idx = 3, n = 10;
    
    // 執行遊戲
    game_controller.executeGame(world_cueballcoor, target_ball_coor, world_childball_coor,
                               collide_coor_project, direct_collide_vector, collide_coor,
                               project_count, idx, n, top, right, left, middle, bottom);
    
    return 0;
}