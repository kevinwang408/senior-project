// ConsoleApplication2.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。

#include <Python.h>
#include "iostream"
#include "windows.h"
#include <fstream>
#include "HRSDK.h"
#include <stdio.h>

/////////////////////////////////////////

#include <iostream>
#include <vector>
#include <cstdio>
#include <math.h>

////////////////////////////////////////

#define ball_diameter 25.6
#define y_pos_correct -15//50 //手臂打擊點修正
#define x_pos_correct 5//50 //手臂打擊點修正
#define z_correct  -8//-18
#define z -100//-20
#define x64
#define top 455
#define middle 152
#define bottom -168
#define left -101
#define right 204
#define cali -3//夾頭裝不上所以+15
#define hitter_a 0

///////////////////////////////////////////////////

#define boundval2 25.8139//2R

using namespace std;

////////////////////////////////////////////////////////////

double INNER_PRODUCT(double, double, double, double);//自訂意涵是
double mag(double, double);
double COS_VAL(double, double, double, double);
double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0);

////////////////////////////////////////////////////////////////////

#ifdef x64
#pragma comment(lib,"C:/Users/wang8/source/repos/ConsoleApplication2/ConsoleApplication2/HRSDK.lib")
#else
#pragma comment(lib, "../../../../lib/x86/HRSDK.lib")
#endif

#ifdef x64
#pragma comment(lib,"C:/Users/wang8/source/repos/cflip/cflip/opencv_world470d.lib")
#else
#pragma comment(lib, "../../../../lib/x86/HRSDK.lib")
#endif
void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {

}//不知道要幹嘛但要就對了

void wait(HROBOT device_id) {
    while (true) {
        if (get_motion_state(device_id) == 1) {
            break;
        }
    }
}

void to_enter_point(HROBOT device_id) {
    //HROBOT device_id = open_connection("192.168.100.45 ", 1, callBack);//169.254.225.26原 172.20.10.3測1
    fstream son_Wcoor;
    fstream childball_num;
    fstream mother_Wcoor;
    fstream recheckcsv;
    son_Wcoor.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/son_Wcoor.csv", ios::in);
    childball_num.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/childball_num.csv", ios::in);
    mother_Wcoor.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/mother_Wcoor.csv", ios::in);//

    char buffer[1024] = { 0 };//建立子球個數存

    int n = 0;
    while (childball_num.getline(buffer, 1024))//將childball資料存入buffer 將整行資料存入buffer 一個字元一個單位存入 最後buffer會存入整行數值 且一次只讀一行
    {
        sscanf_s(buffer, "%d", &n); //讀入撞球個數，以sscanf，存入n %d為整數輸出 即n為子球個數
        //scanf為讀取一段字串所需之資料格式，如%d為讀取此字串中的整數，並將其存入n中
    }
    //cout << n << endl;
    vector<vector<double> > world_childball_coor(n, vector<double>(2, 0));//創立一個第二軸有n個元素(即列有n列) 第一軸為2個元素(即2行) 行初始0
    int i = 0;
    while (son_Wcoor.getline(buffer, 1024))//讀取子球世界座標
    {
        sscanf_s(buffer, "%lf,%lf", &world_childball_coor[i][0], &world_childball_coor[i][1]);//lf數字
        //cout << real_childball_coor[i][0] << ' ' << real_childball_coor[i][1] << ' ' << i << endl;
        //cout << buffer << endl;
        //cout << buffer[i] << endl;
        i++;
    }
    //sscanf_s("123", "%d", &n);
    //cout << n << endl;

    vector<double> world_cueballcoor(2, 0);//mother

    while (mother_Wcoor.getline(buffer, 1024))//讀取母球世界座標
    {
        sscanf_s(buffer, "%lf,%lf", &world_cueballcoor[0], &world_cueballcoor[1]);
    }
    
    std::cout << "母球座標" << endl;
    std::cout << world_cueballcoor[0] <<' ' << world_cueballcoor[1] << endl;


    vector<vector<double> > rel_targetball_hole(6, vector<double>(2, 0));//子球洞口向量 子球*6
    vector<vector<double> > world_hole_coor(6, vector<double>(2, 0));
    world_hole_coor = { {bottom,left},//左下
                      {top,left+10},//左上
                      {middle,left},//左中
                      {top+20,right},//右上
                      {middle,right},//右中
                      {bottom+5,right-10} };//右下

    //假設目標球為子球陣列的第一顆
    vector<vector<double>> test_vector(6 * n, vector<double>(5, 0));//紀錄所有球的相對座標及其位置
    int test = 0;
    vector<double> target_ball_coor(3, 0);//直接定義目標球 1.2為x,y座標 3為距離
    target_ball_coor[2] = 1000;//先設定一個極大值好讓其存入較小值
    /////////////////////////////////////////////////////////////////
    //最近洞口
    ////////////////////////////////////////////////////////////////
    
    
    for (int i = 0; i < 6; i++) {//所有洞口
        for (int j = 0; j < n; j++) {//所有子球
            double test_mag = mag(world_hole_coor[i][0] - world_childball_coor[j][0], world_hole_coor[i][1] - world_childball_coor[j][1]);
            test_vector[test][0] = world_childball_coor[j][0];
            test_vector[test][1] = world_childball_coor[j][1];
            test_vector[test][2] = world_hole_coor[i][0];
            test_vector[test][3] = world_hole_coor[i][1];
            test_vector[test][4] = test_mag;
            test++;
            if (test_mag < target_ball_coor[2]) {
                target_ball_coor[0] = world_childball_coor[j][0];
                target_ball_coor[1] = world_childball_coor[j][1];
                target_ball_coor[2] = test_mag;

            }
        }
    }
    

    ////////////////////////////////////////////////////////////////
    //最近母球
    ////////////////////////////////////////////////////////////////
    /*
    for (int j = 0; j < n; j++) {//所有子球
        double test_mag = mag(world_cueballcoor[0] - world_childball_coor[j][0], world_cueballcoor[1] - world_childball_coor[j][1]);
        test_vector[test][0] = world_childball_coor[j][0];
        test_vector[test][1] = world_childball_coor[j][1];
        test_vector[test][2] = world_cueballcoor[0];
        test_vector[test][3] = world_cueballcoor[1];
        test_vector[test][4] = test_mag;
        test++;

        if (test_mag < target_ball_coor[2]) {
                target_ball_coor[0] = world_childball_coor[j][0];
                target_ball_coor[1] = world_childball_coor[j][1];
                target_ball_coor[2] = test_mag;
        }

    }
    */
    
    ////////////////////////////////////////////////////////////////
    std::cout << "目標球座標" << endl;
    std::cout << target_ball_coor [0]<<' ' << target_ball_coor[1] <<"  " << target_ball_coor[2] << endl;
    /////////////////////////////////////////////////////
    //測洞口準度
    //target_ball_coor[0] = world_hole_coor[1][0]-10;
    //target_ball_coor[1] = world_hole_coor[1][1]+10;
    //////////////////////////////////////////////////////
    ofstream test_vector_output;
    test_vector_output.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/test_vector.csv", ios::out);
    for (int i = 0; i < test; i++) {
        test_vector_output << test_vector[i][0] << "," << test_vector[i][1] << "," << test_vector[i][2] << "," << test_vector[i][3] << "," << test_vector[i][4] << endl;
    }
    test_vector_output.close();

    for (int i = 0; i < 6; i++) {
        rel_targetball_hole[i][0] = world_hole_coor[i][0] - target_ball_coor[0];
        rel_targetball_hole[i][1] = world_hole_coor[i][1] - target_ball_coor[1];
        //cout << rel_targetball_hole[i][0] <<" " << rel_targetball_hole[i][1] << endl;
    }

    vector<vector<double>> targetball_hole_interrupted(100, vector<double>(2, 0));
    vector<vector<double>> inter_detect(6, vector<double>(2, 0));
    vector<vector<double>> available_hole(6, vector<double>(2, 0));
    vector<vector<double>> inter_vector_detect(6, vector<double>(2, 0));
    vector<vector<double>> available_hole_vector(6, vector<double>(2, 0));
    
    for (int i = 0; i < 6; i++) {//先複製出跟世界座標洞相同之vector
        inter_detect[i][0] = world_hole_coor[i][0];
        inter_detect[i][1] = world_hole_coor[i][1];
        inter_vector_detect[i][0] = rel_targetball_hole[i][0];
        inter_vector_detect[i][1] = rel_targetball_hole[i][1];
    }
// above part1

    
    int idx = 0;
    for (int j = 0; j < 6; j++)//所有洞口
    {
        for (int k = 0; k < n; k++)//所有子球
        {
            double PL_distance = dis(world_hole_coor[j][0] - target_ball_coor[0], world_hole_coor[j][1] - target_ball_coor[1], target_ball_coor[0], target_ball_coor[1], world_childball_coor[k][0], world_childball_coor[k][1]);
            if (abs(PL_distance) < boundval2)//計算是否干擾 直線距離值竟 即對洞口與子球間值線路經中，藉由外積積出與其他子球與值線路徑之垂直距離，若小於兩倍半徑則為干擾球
            {

                double mag1 = mag(rel_targetball_hole[j][0], rel_targetball_hole[j][1]);//兩顆子球進洞距離
                double mag2 = mag(world_childball_coor[k][0] - world_hole_coor[j][0], world_childball_coor[k][1] - world_hole_coor[j][1]);//打擊球進洞距離
                if (mag2 < mag1)//對每個子球與洞口
                {
                    targetball_hole_interrupted[idx][0] = world_hole_coor[j][0];
                    targetball_hole_interrupted[idx][1] = world_hole_coor[j][1];
                    inter_detect[j][0] = 10000;//用10000替代掉
                    inter_detect[j][1] = 10000;
                    inter_vector_detect[j][0]=10000;
                    inter_vector_detect[j][1]=10000;
                    std::cout << "不可行洞座標" << endl;
                    std::cout << targetball_hole_interrupted[idx][0] << " " << targetball_hole_interrupted[idx][1] << endl;
                    std::cout << "干擾球座標" << endl;
                    std::cout << world_childball_coor[k][0] << " " << world_childball_coor[k][1] << endl;
                    idx++;
                    break;
                }

            }
        }

    }
    idx = 0;
    std::cout << "可行洞座標與可行洞相較目標子球向量" << endl;
    for (int i = 0; i < 6; i++) {//訂出可行洞及可行向量 小於10000及為可行洞
        //cout << inter_detect[i][0] << " " << inter_detect[i][1] << endl;
        if (inter_detect[i][0] < 9000) {
            available_hole[idx][0] = inter_detect[i][0];
            available_hole[idx][1] = inter_detect[i][1];
            available_hole_vector[idx][0] = inter_vector_detect[i][0];
            available_hole_vector[idx][1] = inter_vector_detect[i][1];
            std::cout << available_hole[idx][0] << " " << available_hole[idx][1] << endl;
            std::cout << available_hole_vector[idx][0] << " " << available_hole_vector[idx][1] << endl;
            idx++;
        }
    }

    std::cout << "可行洞數量:"<<idx << endl;

    vector<vector<double>> unit_hole_vector(6, vector<double>(2, 0));

    for (int i = 0; i < idx; i++) {//將往洞的向量換為單位向量
 
        double hole_vector_mag = mag(available_hole_vector[i][0], available_hole_vector[i][1]);
        unit_hole_vector[i][0] = available_hole_vector[i][0] / hole_vector_mag;
        unit_hole_vector[i][1] = available_hole_vector[i][1] / hole_vector_mag;
        //std::cout << unit_hole_vector[i][0] << " " << unit_hole_vector[i][1] << endl;
        //std::cout << unit_hole_vector[i][0] * unit_hole_vector[i][0] + unit_hole_vector[i][1] * unit_hole_vector[i][1] << endl;
    }
    vector<vector<double>> collide_coor(6, vector<double>(2,0));
    std::cout << "撞擊點座標" << endl;
    for (int i = 0; i < idx; i++) { //找出撞擊點

        collide_coor[i][0] = target_ball_coor[0] - (ball_diameter-5) * unit_hole_vector[i][0];
        collide_coor[i][1] = target_ball_coor[1] - (ball_diameter-5) * unit_hole_vector[i][1];
        std::cout << collide_coor[i][0] << " " << collide_coor[i][1] << endl;
        //std::cout << ball_diameter * unit_hole_vector[i][0] * unit_hole_vector[i][0] +  unit_hole_vector[i][1] * ball_diameter * unit_hole_vector[i][1] << endl;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //以下為直接擊打
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<vector<double>> direct_collide_vector(idx, vector<double>(7, 0));//1.2為撞擊點座標位置x.y 3.4為母球座標位置x,y 5.6為母球至撞擊點向量 7為判斷可行度

    //判斷母球對各洞口直接擊打目標子球的可能性
        
    std::cout << "直接打擊向量及其可行性:" << endl;
    for (int j = 0; j < idx; j++)//所有撞擊點
    {
        direct_collide_vector[j][0] = collide_coor[j][0];
        direct_collide_vector[j][1] = collide_coor[j][1];
        direct_collide_vector[j][2] = world_cueballcoor[0];
        direct_collide_vector[j][3] = world_cueballcoor[1];
        direct_collide_vector[j][4] = collide_coor[j][0] - world_cueballcoor[0];
        direct_collide_vector[j][5] = collide_coor[j][1] - world_cueballcoor[1];

        for (int k = 0; k < n; k++)//所有子球
        {
            double PL_distance = dis(collide_coor[j][0] - world_cueballcoor[0], collide_coor[j][1] - world_cueballcoor[1], world_cueballcoor[0], world_cueballcoor[1], world_childball_coor[k][0], world_childball_coor[k][1]);
            if (abs(PL_distance) < boundval2)//計算是否干擾 直線距離值竟 即對洞口與子球間值線路經中，藉由外積積出與其他子球與值線路徑之垂直距離，若小於兩倍半徑則為干擾球
            {

                double mag1 = mag(collide_coor[j][0] - world_cueballcoor[0], collide_coor[j][1] - world_cueballcoor[1]);//兩顆子球進洞距離
                double mag2 = mag(world_childball_coor[k][0] - collide_coor[j][0], world_childball_coor[k][1] - collide_coor[j][1]);//打擊球進洞距離
                double mag3 = mag(world_childball_coor[k][0] - target_ball_coor[0], world_childball_coor[k][1] - target_ball_coor[1]);//此子球與目標子球之距離若小於20可認定其為目標子球本身
                if (mag3 > 20) {
                    if (mag2 < mag1)//對每個子球與洞口
                    {
                        direct_collide_vector[j][6] = 10000;
                        std::cout << "路徑干擾不通過: "<< j+1 << endl;
                        break;
                    }
                }
            }
        }
        if (direct_collide_vector[j][2] < 9000) {
            double angle1 = acos(COS_VAL(direct_collide_vector[j][4], direct_collide_vector[j][5], target_ball_coor[0] - collide_coor[j][0], target_ball_coor[1] - collide_coor[j][1])) * 180 / 3.1415926;
            if (angle1 > 70) {//夾角太大(相較原點)會導致自身成為干擾球
                direct_collide_vector[j][6] = 10000;
                std::cout << "打擊角度不通過: " << j + 1 << endl;
            }
            else {

            }
        }
        std::cout << direct_collide_vector[j][0] << " " << direct_collide_vector[j][1] << " " << direct_collide_vector[j][2] << endl;
    }
    
    ofstream direct_collide;
    direct_collide.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/direct_collide.csv", ios::out);
    for (int i = 0; i < idx; i++) {
        direct_collide << direct_collide_vector[i][0] << "," << direct_collide_vector[i][1] << "," << direct_collide_vector[i][2] << "," << direct_collide_vector[i][3] << "," << direct_collide_vector[i][4] << "," << direct_collide_vector[i][5] << "," << direct_collide_vector[i][6] << endl;
    }
    direct_collide.close();
    int direct_num = idx;;
    std::cout << "直接擊打數量:" << direct_num << endl;


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //以下為彈射
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    vector<vector<double>> collide_coor_project(idx * 4, vector<double>(8, 0));
    int project_count = 0;
    double project_flip_ratio = 0;
    //std::cout << "撞擊點座標測試" << endl;
    for (int i = 0; i < idx; i++) {//每個撞擊點 對每個邊做投影 一為投影點x 二為投影點y 三為母球與投影x向量 四為母球與投影y向量 5為母球至彈射點x向量 6為母球至彈射點y向量 7為撞擊點x 8為撞擊點y
        //投影頂部
        double rel_top = top - collide_coor[i][0] - 13;//撞擊點球半徑;
        double cue_rel_top = top - world_cueballcoor[0]-13;
        collide_coor_project[project_count][0] = collide_coor[i][0] + rel_top + cue_rel_top;
        collide_coor_project[project_count][1] = collide_coor[i][1];
        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - world_cueballcoor[0];
        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - world_cueballcoor[1];
        project_flip_ratio = cue_rel_top / collide_coor_project[project_count][2];
        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
        collide_coor_project[project_count][6] = collide_coor[i][0];
        collide_coor_project[project_count][7] = collide_coor[i][1];
        //cout << collide_coor_project[project_count][6] << " " << collide_coor_project[project_count][7] << endl;
        project_count++;
        //投影左邊
        double rel_left = left - collide_coor[i][1] + 13;
        double cue_rel_left = left - world_cueballcoor[1]+13;
        collide_coor_project[project_count][0] = collide_coor[i][0];
        collide_coor_project[project_count][1] = collide_coor[i][1] + rel_left + cue_rel_left;
        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - world_cueballcoor[0];
        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - world_cueballcoor[1];
        project_flip_ratio = cue_rel_left / collide_coor_project[project_count][3];
        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
        collide_coor_project[project_count][6] = collide_coor[i][0];
        collide_coor_project[project_count][7] = collide_coor[i][1];
        //cout << collide_coor_project[project_count][6] << " " << collide_coor_project[project_count][7] << endl;
        project_count++;
        //投影底部
        double rel_bottom = bottom - collide_coor[i][0] + 13;
        double cue_rel_bottom = bottom - world_cueballcoor[0]+13;
        collide_coor_project[project_count][0] = collide_coor[i][0] + rel_bottom + cue_rel_bottom;
        collide_coor_project[project_count][1] = collide_coor[i][1];
        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - world_cueballcoor[0];
        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - world_cueballcoor[1];
        project_flip_ratio = cue_rel_bottom / collide_coor_project[project_count][2];
        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
        collide_coor_project[project_count][6] = collide_coor[i][0];
        collide_coor_project[project_count][7] = collide_coor[i][1];
        //cout << collide_coor_project[project_count][6] << " " << collide_coor_project[project_count][7] << endl;
        project_count++;
        //投影右邊
        double rel_right = right - collide_coor[i][1] - 13;
        double cue_rel_right = right - world_cueballcoor[1]-13;
        collide_coor_project[project_count][0] = collide_coor[i][0];
        collide_coor_project[project_count][1] = collide_coor[i][1] + rel_right+ cue_rel_right;
        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - world_cueballcoor[0];
        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - world_cueballcoor[1];
        project_flip_ratio = cue_rel_right / collide_coor_project[project_count][3];
        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
        collide_coor_project[project_count][6] = collide_coor[i][0];
        collide_coor_project[project_count][7] = collide_coor[i][1];
        //cout << collide_coor_project[project_count][6] << " " << collide_coor_project[project_count][7] << endl;
        project_count++;

    }
    std::cout <<"一次投影後目標點數量:" << project_count << endl;
    //above to 200row part2
    vector<vector<double>> one_flip_point(project_count, vector<double>(7,0)); //3為彈射前干擾 4為彈射後干擾 5為角度干擾 6為彈射點在洞口上 7為檢測後此彈射點是否可行，0即為可行
    vector<vector<double>> one_flip_vector(project_count, vector<double>(7, 0));//1為母球至彈射點x向量 2為母球至彈射點y向量 3為彈射點至撞擊點向量x 4為彈射點至撞擊點向量y 5,6為撞擊點座標x,y 7為夾角
    for (int i = 0; i < project_count; i++) {
        one_flip_point[i][0] = world_cueballcoor[0] + collide_coor_project[i][4];
        one_flip_point[i][1] = world_cueballcoor[1] + collide_coor_project[i][5];
        one_flip_vector[i][0] = collide_coor_project[i][4];
        one_flip_vector[i][1] = collide_coor_project[i][5];
        one_flip_vector[i][2] = collide_coor_project[i][6] - one_flip_point[i][0];
        one_flip_vector[i][3] = collide_coor_project[i][7] - one_flip_point[i][1];
        one_flip_vector[i][4] = collide_coor_project[i][6];
        one_flip_vector[i][5] = collide_coor_project[i][7];
    }
    
    /*
    for (int i = 0; i < project_count; i++) {
        one_flip_vector[i][0] = collide_coor_project[i][4];
        one_flip_vector[i][1] = collide_coor_project[i][5];
        one_flip_vector[i][2] = collide_coor_project[i][6] - one_flip_point[i][0];
        one_flip_vector[i][3] = collide_coor_project[i][7] - one_flip_point[i][1];
    }
    */
    //找干擾球
    int detect = 0;
    int detect2 = 0;
    for (int i = 0; i < project_count; i++)//
    {
        for (int k = 0; k < n; k++)//所有子球
        {
            double PL_distance1 = dis(one_flip_vector[i][0], one_flip_vector[i][1], world_cueballcoor[0], world_cueballcoor[1], world_childball_coor[k][0], world_childball_coor[k][1]);
            double PL_distance2 = dis(one_flip_vector[i][2], one_flip_vector[i][3], one_flip_point[i][0], one_flip_point[i][1], world_childball_coor[k][0], world_childball_coor[k][1]);
            if (abs(PL_distance1) < boundval2)//計算是否干擾 直線距離值勁 在此為母球以及彈射點 即對洞口與子球間值線路經中，藉由外積積出與其他子球與值線路徑之垂直距離，若小於兩倍半徑則為干擾球
            {

                double mag1 = mag(one_flip_vector[i][0], one_flip_vector[i][1]);//兩顆子球進洞距離
                double mag2 = mag(world_childball_coor[k][0] - one_flip_point[i][0], world_childball_coor[k][1] - one_flip_point[i][1]);//打擊球進洞距離
                if (mag2 < mag1)//對每個子球與洞口
                {
                    one_flip_point[i][2] = 10000;
                    //one_flip_vector[i][0] = 10000;
                    //one_flip_vector[i][1] = 10000;
                    //one_flip_vector[i][2] = 10000;
                    //one_flip_vector[i][3] = 10000;

                    std::cout<<"第幾個軌道:" << i <<" detect1=" << detect<< " mag1="<<mag1<<" mag2="<<mag2 << endl;
                    detect++;
                    break;
                }


            }else if(abs(PL_distance2) < boundval2)//計算是否干擾 直線距離值勁 在此為母球以及彈射點 即對洞口與子球間值線路經中，藉由外積積出與其他子球與值線路徑之垂直距離，若小於兩倍半徑則為干擾球
            {

                double mag1 = mag(one_flip_vector[i][2], one_flip_vector[i][3]);
                //cout << "mag1=" << mag1 << endl;
                double mag2 = mag(world_childball_coor[k][0] - (one_flip_vector[i][4]- one_flip_vector[i][2]), world_childball_coor[k][1] - (one_flip_vector[i][5]- one_flip_vector[i][3]));//4,5為撞擊點 在此為到達彈射點距離
                double mag3 = mag(world_childball_coor[k][0] - target_ball_coor[0], world_childball_coor[k][1] - target_ball_coor[1]);//此子球與目標子球之距離若小於20可認定其為目標子球本身
                if (mag3 > 20) {
                    if (mag2 < mag1)
                    {
                        one_flip_point[i][3] = 10000;
                        std::cout<<"第幾個軌道:"<<i << " detect2=" << detect2 << " mag1=" << mag1 << " mag2=" << mag2 << endl;
                        detect2++;
                        break;
                    }
                }
                


            }
        }

    }
    project_count = 0;
    
    for (int i = 0; i < idx; i++) {//找出目標球本身即為干擾球 彈射至撞擊點向量與撞擊點至目標子球向量夾角小於110度
        double angle1 = acos(COS_VAL(one_flip_vector[project_count][2], one_flip_vector[project_count][3], target_ball_coor[0] - collide_coor[i][0], target_ball_coor[1] - collide_coor[i][1])) * 180 / 3.1415926;
        one_flip_vector[project_count][6] = angle1;
        //std::cout << angle1 << endl;
        if (angle1 > 80) {//夾角太大(相較原點)會導致自身成為干擾球
            one_flip_point[project_count][4] = 10000;
            //std::cout << "angle!" << endl;
        }
        project_count++;
        //std::cout << project_count << endl;
        double angle2 = acos(COS_VAL(one_flip_vector[project_count][2], one_flip_vector[project_count][3], target_ball_coor[0] - collide_coor[i][0], target_ball_coor[1] - collide_coor[i][1])) * 180 / 3.1415926;
        one_flip_vector[project_count][6] = angle2;
        //std::cout << angle2 << endl;
        if (angle2 > 80) {//夾角太大會導致自身成為干擾球
            one_flip_point[project_count][4] = 10000;
            //std::cout << "angle!" << endl;
        }
        project_count++;
        std::cout << project_count << endl;
        double angle3 = acos(COS_VAL(one_flip_vector[project_count][2], one_flip_vector[project_count][3], target_ball_coor[0] - collide_coor[i][0], target_ball_coor[1] - collide_coor[i][1])) * 180 / 3.1415926;
        one_flip_vector[project_count][6] = angle3;
        //std::cout << angle3 << endl;
        if (angle3 > 80) {//夾角太大會導致自身成為干擾球
            one_flip_point[project_count][4] = 10000;
            //std::cout << "angle!" << endl;
        }
        project_count++;
        //std::cout << project_count << endl;
        double angle4 = acos(COS_VAL(one_flip_vector[project_count][2], one_flip_vector[project_count][3], target_ball_coor[0] - collide_coor[i][0], target_ball_coor[1] - collide_coor[i][1])) * 180 / 3.1415926;
        one_flip_vector[project_count][6] = angle4;
        //std::cout << angle4 << endl;
        if (angle4 > 80) {//夾角太大會導致自身成為干擾球
            one_flip_point[project_count][4] = 10000;
            //std::cout << "angle!" << endl;
        }
        project_count++;
        //std::cout << project_count << endl;
    }
    
    for (int i = 0; i < idx * 4; i++) {//若是彈射點座標位於洞口處附近則無法通過
        if (one_flip_point[i][1]>-92 and one_flip_point[i][1]<-83) {
            if (one_flip_point[i][0]<-148 and one_flip_point[i][0]>-188) {
                one_flip_point[i][5] = 10000;
            }
            else if (one_flip_point[i][0] < 172 and one_flip_point[i][0]>132) {
                one_flip_point[i][5] = 10000;
            }
            else if (one_flip_point[i][0] < 485 and one_flip_point[i][0]>445) {
                one_flip_point[i][5] = 10000;
            }
        }
        else if (one_flip_point[i][1] < 196 && one_flip_point[i][1]>186) {
            if (one_flip_point[i][0]<-148 and one_flip_point[i][0]>-188) {
                one_flip_point[i][5] = 10000;
            }
            else if (one_flip_point[i][0] < 172 and one_flip_point[i][0]>132) {
                one_flip_point[i][5] = 10000;
            }
            else if (one_flip_point[i][0] < 485 and one_flip_point[i][0]>445) {
                one_flip_point[i][5] = 10000;
            }
        }
    }
  
    for (int i = 0; i < idx*4; i++) {//對所有彈射向量整合，找出可行彈射做計算
        for (int j = 0; j < 6; j++) {
            if (one_flip_point[i][j] > 9000) {
                one_flip_point[i][6] = 1;
                break;
            }
        }
    }


    
    
    ofstream flip_once_point;
    flip_once_point.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/flip_once_point.csv", ios::out);
    for (int i = 0; i < project_count; i++) {
        flip_once_point << one_flip_point[i][0] << "," << one_flip_point[i][1] << "," << one_flip_point[i][2] << "," << one_flip_point[i][3] << "," << one_flip_point[i][4] << "," << one_flip_point[i][5] << "," << one_flip_point[i][6] << endl;
    }
    flip_once_point.close();

    ofstream flip_once_vector;
    flip_once_vector.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/flip_once_vector.csv", ios::out);
    for (int i = 0; i < project_count; i++) {
        flip_once_vector << one_flip_vector[i][0] << "," << one_flip_vector[i][1] << "," << one_flip_vector[i][2]<<"," << one_flip_vector[i][3]<< "," << one_flip_vector[i][4]<< "," << one_flip_vector[i][5] << "," << one_flip_vector[i][6] << endl;
    }
    flip_once_vector.close();

    
    int find_final = 0;//用以判斷是否找到可行點 0為沒有 1為有

    vector<double> final_coor(2, 0);//最終打擊點座標 即機械手臂要抵達之座標
    vector<double> final_coor_unit(2, 0);//母球往目標點之單位向量
    double cue_target_dis = 10000;
    double target_hole_dis = 0;
    for (int i = 0; i < idx; i++) {//值接擊打
        if (direct_collide_vector[i][6] == 0) {
            if (mag(direct_collide_vector[i][4], direct_collide_vector[i][5]) < cue_target_dis) {
                double final_mag = mag(direct_collide_vector[i][4], direct_collide_vector[i][5]);
                final_coor_unit[0] = direct_collide_vector[i][4] / final_mag;
                final_coor_unit[1] = direct_collide_vector[i][5] / final_mag;
                final_coor[0] = world_cueballcoor[0] - (ball_diameter + cali) * final_coor_unit[0];
                final_coor[1] = world_cueballcoor[1] - (ball_diameter + cali) * final_coor_unit[1];
                find_final = 1;//找到
                cue_target_dis = final_mag;

                std::cout << "找到" << endl;
                std::cout << "打擊座標點:" << final_coor[0] << " , " << final_coor[1] << endl;

            } 
        }
    }

    if (find_final == 2) {//彈射
        for (int i = 0; i < idx * 4; i++) {
            if (one_flip_point[i][6] == 0) {
                if ((mag(one_flip_vector[i][0], one_flip_vector[i][1]) + mag(one_flip_vector[i][3], one_flip_vector[i][4])) < cue_target_dis) {

                    double test_mag = mag(one_flip_vector[i][0], one_flip_vector[i][1]) + mag(one_flip_vector[i][3], one_flip_vector[i][4]);
                    double final_mag = mag(one_flip_vector[i][0], one_flip_vector[i][1]);
                    final_coor_unit[0] = one_flip_vector[i][0] / final_mag;
                    final_coor_unit[1] = one_flip_vector[i][1] / final_mag;
                    final_coor[0] = world_cueballcoor[0] - (ball_diameter + cali) * final_coor_unit[0];
                    final_coor[1] = world_cueballcoor[1] - (ball_diameter + cali) * final_coor_unit[1];
                    find_final = 1;//找到
                    cue_target_dis = test_mag;
                    std::cout << "找到" << endl;
                    std::cout << "打擊座標點:" << final_coor[0] << " , " << final_coor[1] << endl;

                }
                
            }
        }
    }
    //part3
    if (find_final == 0) {
        std::cout << "沒找到，撞上去喽" << endl;
        vector<double> no_way(2, 0);
        no_way[0] = target_ball_coor[0] - world_cueballcoor[0];
        no_way[1] = target_ball_coor[1] - world_cueballcoor[1];
        double final_mag = mag(no_way[0], no_way[1]);
        final_coor_unit[0] = no_way[0] / final_mag;
        final_coor_unit[1] = no_way[1] / final_mag;
        final_coor[0] = world_cueballcoor[0] - (ball_diameter + cali) * final_coor_unit[0];
        final_coor[1] = world_cueballcoor[1] - (ball_diameter + cali) * final_coor_unit[1];
        find_final = 1;//找到

    }

    std::cout << "力道控制前" << final_coor[0] << " " << final_coor[1] << endl;

    ofstream final_hit_coor;
    final_hit_coor.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/final_coor.csv", ios::out);
    final_hit_coor << final_coor[0] <<","<< final_coor[1] << endl;
    final_hit_coor.close();

    double vector_x = final_coor_unit[0];//母球至目標點單位向量x分量
    double vector_y = final_coor_unit[1];//母球至目標點單位向量y分量
    double vector_HIT_PATH[3] = { vector_x, vector_y, 0 };
    double vector_TCP[3] = { 0,-1,0 };//不清楚 似乎用來計算與y軸之垂直夾腳 應該是指原始的打擊方向為0.1.0
    //0.-1.0是我打擊器方向 找出與打擊器的夾角再做轉動
    double inner_product = 0;
    double sum = 0;
    std::cout << "打擊方向向量" << vector_x << "," << vector_y << endl;
    for (int i = 0; i < 3; i++)
    {
        sum = vector_HIT_PATH[i] * vector_TCP[i];
        inner_product += sum;
    }
    std::cout << "inner_product:" << inner_product << std::endl;
    double theta = abs(acos(inner_product));
    std::cout << theta << std::endl;

    theta = theta * 180 / 3.1415926;
    std::cout << "theta" << theta << std::endl;
    std::cout << "距離" << cue_target_dis << endl;
    int theta_cali = 0;
    
    if (cue_target_dis+ target_ball_coor[3] < 200) {
        std::cout << "很近"  << endl;
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis+target_ball_coor[3] >= 200 && cue_target_dis+target_ball_coor[3] < 400) {
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else {
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊
    }
    
    final_coor[0]= final_coor[0] + (theta_cali) * final_coor_unit[0];
    final_coor[1] = final_coor[1] + (theta_cali)*final_coor_unit[1];
    
    std::cout << "力道控制後" << final_coor[0] << " " << final_coor[1] << endl;
    double pos_cueball[6] = { 0 };
    double origin_point[6] = { 90,0,0,0,-90,0 };

    ofstream pythonplot;
    pythonplot.open("C:/Users/wang8/source/repos/C++candidate_path/C++candidate_path/circle.csv", ios::out);
    //pythonplot.open("C:/Users/wang8/source/repos/cflip/cflip/pythonplot.xlsx", ios::out);
    pythonplot << top+20 << "," << right << endl;
    pythonplot << top << "," << left + 10 << endl;
    pythonplot << middle << "," << right << endl;
    pythonplot << middle << "," << left << endl;
    pythonplot << bottom+5 << "," << right - 10 << endl;
    pythonplot << bottom << "," << left << endl;
    pythonplot << world_cueballcoor[0] << "," << world_cueballcoor[1] << endl;
    pythonplot << target_ball_coor[0] << "," << target_ball_coor[1] << endl;
    for (int i = 0; i < idx; i++) { //找出撞擊點
        pythonplot << collide_coor[i][0] << "," << collide_coor[i][1] << endl;
    }
    pythonplot.close();
    double final_coor_x = 0;
    double final_coor_y = 0;
    final_coor_x = final_coor[0];
    final_coor_y = final_coor[1];
    if (final_coor_x >= 455+ hitter_a) {
        final_coor_x = 455+ hitter_a;
    }
    else if (final_coor_x <= -158- hitter_a) {
        final_coor_x = -158- hitter_a;
    }
    if (final_coor_y >= 195+ hitter_a) {
        final_coor_y = 195+ hitter_a;
    }
    else if (final_coor_y <= -95- hitter_a) {
        final_coor_y = -95- hitter_a;
    }
    final_coor[0] = final_coor_x;
    final_coor[1] = final_coor_y;

    double origin_point_inner[6] = { 90,0,15,0,-105,0 };

    if (vector_x > 0)
    {
        //pos_cueball = { x_cueball_coor, y_cueball_coor, z_cueball_coor ,0,0,-theta};
        *pos_cueball = final_coor[0];//打擊點 原來陣列這樣打ㄛ
        *(pos_cueball + 1) = final_coor[1];
        *(pos_cueball + 2) = z;
        *(pos_cueball + 3) = 0;
        *(pos_cueball + 4) = 0;//-25
        *(pos_cueball + 5) = -90+theta;//繞Z軸旋轉-theta度 為打擊點  以原廠位置的工具座標為基準 -90+theta
        std::cout << "A:theta" << theta << std::endl;
        ptp_pos(device_id, 1, pos_cueball);
        wait(device_id);
        *(pos_cueball + 2) = z_correct;
        wait(device_id);
        ptp_pos(device_id, 1, pos_cueball);
        wait(device_id);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        Sleep(500);
        set_digital_output(device_id, 16, true);
        Sleep(500);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        wait(device_id);
        //ptp_axis(device_id,1, origin_point);
        ptp_axis(device_id, 0, origin_point_inner);
        wait(device_id);
        std::cout << "?" << std::endl;

    }
    else//x向量並非大於0
    {
        //pos_cueball = { x_cueball_coor, y_cueball_coor, z_cueball_coor ,0,0,theta };
        *pos_cueball = final_coor[0];
        *(pos_cueball + 1) = final_coor[1];
        *(pos_cueball + 2) = z;
        *(pos_cueball + 3) =0 ;
        *(pos_cueball + 4) = 0;//-25
        *(pos_cueball + 5) = -90-theta;//-90-thteta
        std::cout << "B:theta" << theta << std::endl;
        ptp_pos(device_id, 1, pos_cueball);
        wait(device_id);
        *(pos_cueball + 2) = z_correct;
        wait(device_id);
        ptp_pos(device_id, 1, pos_cueball);
        wait(device_id);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        Sleep(500);
        set_digital_output(device_id, 16, true);
        Sleep(500);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        wait(device_id);

        //ptp_axis(device_id,1, origin_point);
        ptp_axis(device_id, 0, origin_point_inner);
        wait(device_id);
        std::cout << "!" << std::endl;
         
    }
    std::cout << world_cueballcoor[0] << "," << world_cueballcoor[1] << endl;
    std::cout << target_ball_coor[0] << "," << target_ball_coor[1] << endl;
    std::cout << final_coor[0] << "," << final_coor[1] << endl;
    std::cout << "預設位置:" << pos_cueball[0]<<" "<< pos_cueball[1]<<" "<< pos_cueball[2]<<" "<< pos_cueball[3]<<" "<< pos_cueball[4]<<" "<< pos_cueball[5] << endl;
    

    
}




int main() {
    
    FILE* fp1 = fopen("D:/hiwin/vscode_python/circle_detect_640.py", "r");
 
    Py_Initialize();

    PyRun_AnyFile(fp1, "D:/hiwin/vscode_python/circle_detect_640.py");
    std::cout << "fuck";
    Py_Finalize();

    fclose(fp1);
    
    char sdk_ver[50];
    char hrss_ver[50];
    get_hrsdk_version(sdk_ver);

    std::cout << "SDK version: " << sdk_ver << std::endl;
    HROBOT device_id = open_connection("192.168.100.45", 1, callBack);//169.254.225.26

    set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 14, true);//設定輸出 似乎是打
    set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊
    std::cout << "connect" << endl;

    double origin_point[6] = { 90,0,15,0,-105,0 };
    set_home_point(device_id, origin_point);
    double test_point[6] = { 40,40,z,0,-25,-90 };

    //ptp_pos(device_id, 1, test_point);
    //Sleep(5000);
      
    double origin_point_inner[6] = { 90,0,15,0,-105,0 };

    if (device_id >= 0) {
        set_operation_mode(device_id, 1);
        set_override_ratio(device_id, 60);
        set_acc_dec_ratio(device_id, 45);
        get_hrss_version(device_id, hrss_ver);
        std::cout << "HRSS version: " << hrss_ver << std::endl;
        std::cout << "connect successful." << std::endl;
       
        //LinExample(device_id);


        to_enter_point(device_id);
        //square(device_id);
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊
        std::cout << "\n Press \"Enter\" key to quit the program." << std::endl;
        std::cin.get();
        disconnect(device_id);
    }
    else {
        std::cout << "connect failure." << std::endl;
    }
    
    return 0;
}


///////////////////////////////////////////////////////


double INNER_PRODUCT(double a, double b, double c, double d)//內積
{
    return a * c + b * d;
}
double mag(double a, double b)//純量
{
    return sqrt(a * a + b * b);
}
double COS_VAL(double a, double b, double c, double d)//cos夾角
{
    return (INNER_PRODUCT(a, b, c, d) / (mag(a, b) * mag(c, d)));
}
double dis(double vec_x, double vec_y, double pass_x, double pass_y, double test_x, double test_y)//計算各子球對一指定打擊子球之打擊路徑距離 vec_x=b vec_y=-a
{
    double c = vec_y * pass_x - vec_x * pass_y;//vec_y為負的 故換邊為正
    double distance = (vec_y * test_x - vec_x * test_y - c) / sqrt(vec_x * vec_x + vec_y * vec_y);//各子球向量對 洞口到指定子球向量外積 最後除上指定向量之量值
    return distance;
}
//////////////////////////////////////////////////////////////////////