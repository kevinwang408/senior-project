//#include <Python.h>
#include "iostream"
#include "windows.h"
#include <fstream>
#include "HRSDK.h"
/////////////////////////////////////////
#include <iostream>
#include <vector>
#include <cstdio>
#include <math.h>
////////////////////////////////////////
#define ball_diameter 25.6
#define y_pos_correct -15//50 //手臂打擊點修正
#define x_pos_correct 5//50 //手臂打擊點修正
#define z_correct  -208
#define z -200//-20
#define x64
#define top 455
#define middle 152
#define bottom -168
#define left -101
#define right 204
#define cali -1//夾頭裝不上所以+15
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

void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {
}//不知道要幹嘛但要就對了
void wait(HROBOT device_id) {//閒置時下一步
    while (true) {
        if (get_motion_state(device_id) == 1) {
            break;
        }
    }
}

void to_enter_point(HROBOT device_id)
{
    /////////////////////////////////////////////////////////////////////////////////////
    fstream son_Wcoor;
    fstream childball_num;
    fstream mother_Wcoor;
    fstream recheckcsv;

    son_Wcoor.open("C:/Users/wang8/source/repos/C++repeat/C++repeat/son_Wcoor.csv", ios::in);
    childball_num.open("C:/Users/wang8/source/repos/C++repeat/C++repeat/childball_num.csv", ios::in);
    mother_Wcoor.open("C:/Users/wang8/source/repos/C++repeat/C++repeat/mother_Wcoor.csv", ios::in);//

    char buffer[1024] = { 0 };//建立子球個數存

    int n = 0;
    while (childball_num.getline(buffer, 1024))//將childball資料存入buffer
    {
        sscanf_s(buffer, "%d", &n); //讀入撞球個數，以sscanf，存入n
    }
    cout << typeid(childball_num).name() << endl;
    cout << buffer[0] << endl;

    vector<vector<double> > real_childball_coor(n, vector<double>(2, 0));//輸入兩個直 初始0
    int i = 0;
    while (son_Wcoor.getline(buffer, 1024))//讀取子球世界座標
    {
        sscanf_s(buffer, "%lf,%lf", &real_childball_coor[i][0], &real_childball_coor[i][1]);//lf數字
        cout << real_childball_coor[i][0] << ' ' << real_childball_coor[i][1] << ' ' << i << endl;
        i++;
    }

    vector<double> real_cueballcoor(2, 0);//mother

    while (mother_Wcoor.getline(buffer, 1024))//讀取母球世界座標
    {
        sscanf_s(buffer, "%lf,%lf", &real_cueballcoor[0], &real_cueballcoor[1]);
    }

    vector<vector<double> > rel_hole_childball(n * 6, vector<double>(2, 0));//子球母球向量 子球*6
    vector<vector<double> > real_holecoor(6, vector<double>(2, 0));
    real_holecoor = { {bottom,left},//左下
                      {top,left + 10},//左上
                      {middle,left},//左中
                      {top+20,right},//右上
                      {middle,right},//右中
                      {bottom+5,right - 10} };//右下

    int idx = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            rel_hole_childball[idx][0] = real_childball_coor[i][0] - real_holecoor[j][0];
            rel_hole_childball[idx][1] = real_childball_coor[i][1] - real_holecoor[j][1];//取得 hole-ball 向量放入rel_hole_ball
            idx++;
        }
    }

    vector<vector<double> > childball_interupted(36 * n * n, vector<double>(2, 0));
    int counter = 0;
    for (int i = 0; i < n; i++)//所有目標子球
    {
        for (int j = 0; j < 6; j++)//所有洞口
        {
            for (int k = 0; k < n; k++)//所有其他子球
            {
                double PL_distance = dis(real_childball_coor[i][0] - real_holecoor[j][0], real_childball_coor[i][1] - real_holecoor[j][1], real_holecoor[j][0], real_holecoor[j][1], real_childball_coor[k][0], real_childball_coor[k][1]);
                if (abs(PL_distance) < boundval2)//計算是否干擾 直線距離值竟
                {
                    double mag1 = mag(real_childball_coor[i][0] - real_holecoor[j][0], real_childball_coor[i][1] - real_holecoor[j][1]);
                    double mag2 = mag(real_childball_coor[k][0] - real_holecoor[j][0], real_childball_coor[k][1] - real_holecoor[j][1]);//打擊球進洞距離
                    if (mag2 < mag1)
                    {
                        childball_interupted[counter][0] = real_childball_coor[i][0] - real_holecoor[j][0];
                        childball_interupted[counter][1] = real_childball_coor[i][1] - real_holecoor[j][1];
                        counter++;
                    }
                }
            }

        }
    }

    for (int j = 0; j < counter; j++)
    {
        for (int i = 0; i < 6 * n; i++)
        {
            if (rel_hole_childball[i][0] == childball_interupted[j][0] && rel_hole_childball[i][1] == childball_interupted[j][1])
            {
                rel_hole_childball[i][0] = 10000;//將會干擾的向量設為極大 剩下的rel_hole_childball陣列中 如果子球與洞口間無干擾的向量則其大小可確保為<10000
                rel_hole_childball[i][1] = 10000;
            }
        }
    }

    vector<double> hole_childball_mag(6 * n, 0);

    int selected_ball_counter = 0;
    for (int i = 0; i < 6 * n; i++)
    {
        hole_childball_mag[i] = mag(rel_hole_childball[i][0], rel_hole_childball[i][1]);
        //找出篩選後的球洞距離
        if (hole_childball_mag[i] < 10000)//超過10000即表示有干擾球
        {
            //cout << hole_childball_mag[i] << endl;

            selected_ball_counter++;//找出沒有干擾球的部分 意即可以打之子球向量的數目
        }
    }
    double min = 0, temp = 0;
    for (int i = 0; i < 6 * n; i++)//sort magnitude 由小到大
    {
        min = hole_childball_mag[i];
        for (int j = i + 1; j < 6 * n; j++)
        {
            if (hole_childball_mag[j] < min)
            {
                min = hole_childball_mag[j];
                temp = hole_childball_mag[i];
                hole_childball_mag[i] = hole_childball_mag[j];
                hole_childball_mag[j] = temp;

            }
        }
    }
    
    std::cout << 221 << endl;
    vector<vector<double> > selected_childball_hole(selected_ball_counter, vector<double>(2, 0));
    int count = 0;

    for (int j = 0; j < selected_ball_counter; j++)//被選中球
    {
        for (int i = 0; i < 6 * n; i++)//所有子球相對洞口向量
        {

            if (mag(rel_hole_childball[i][0], rel_hole_childball[i][1]) == hole_childball_mag[j])
            {   //已找出最接近洞口之子球的mag，找出其座標
                //找出與被篩選出向量大小一樣者求得被篩選出之真實相對向量
                //因為排序過所以會先找到距離較小者

                selected_childball_hole[count][0] = rel_hole_childball[i][0];//取出所有符合篩選目標之相對向量
                selected_childball_hole[count][1] = rel_hole_childball[i][1];
                count++;//找到才依序放入 距離由小而大
                //cout << selected_childball_hole[count][0] <<' '<< selected_childball_hole[count][1];
            }

        }
    }
 
    vector<double> target_childball_coor(2, 0);
    vector<double> target_holecoor(2, 0);
    vector<double> rel_cueball_target(2, 0);
    vector<double> final_childball_coor(2, 0);
    vector<double> final_hole_coor(2, 0);
    vector<vector<double> > target_other_vec(n, vector<double>(2, 0));
    vector<vector<double> > rel_child_targetchild(n, vector<double>(2, 0));

    vector<vector<double> > childresult(100, vector<double>(2, 0));
    vector<vector<double> > holeresult(100, vector<double>(2, 0));
    int resultidx = 0;
    std::cout << 261 << endl;
    //找出子球母球間有無干擾 沒有就可打
    for (int k = selected_ball_counter - 1; k >= 0; k--)//selected_ball_counter陣列中有所有候選子球 由距離最大的開始找 若找到更小的便變為更小的量值
    {
        for (int i = 0; i < n; i++)//子球
        {
            for (int j = 0; j < 6; j++)//洞口
            {
                if ((real_childball_coor[i][0] - real_holecoor[j][0]) == selected_childball_hole[k][0] && real_childball_coor[i][1] - real_holecoor[j][1] == selected_childball_hole[k][1])
                {
                    //如果相減之相對向量與篩檢出之相對向量符合
                    //第一層運算通過 取出目標之真實洞口、子球座標
                    target_childball_coor[0] = real_childball_coor[i][0];
                    target_childball_coor[1] = real_childball_coor[i][1];//取出目標之真實洞口、子球座標
                    //cout << real_childball_coor[i][0] << ' ' << real_childball_coor[i][1] << endl;
                    //cout << target_childball_coor[0] << ' ' << target_childball_coor[1] << endl;

                    for (int g = 0; g < n; g++)//被選中子球與其他子球之相對向量
                    {
                        if ((real_childball_coor[g][0] - target_childball_coor[0]) == 0)
                        {
                            rel_child_targetchild[g][0] = 10000;//不能直接continue啦==
                            rel_child_targetchild[g][1] = 10000;
                        }
                        else
                        {
                            rel_child_targetchild[g][0] = real_childball_coor[g][0] - target_childball_coor[0];
                            rel_child_targetchild[g][1] = real_childball_coor[g][1] - target_childball_coor[1];
                            //取出篩選出子球與其他子球的向量 
                        }
                    }

                    target_holecoor[0] = real_holecoor[j][0];
                    target_holecoor[1] = real_holecoor[j][1];
                    rel_cueball_target[0] = real_cueballcoor[0] - target_childball_coor[0];
                    rel_cueball_target[1] = real_cueballcoor[1] - target_childball_coor[1];//母球與篩選出子球之向量
                    //cout << target_childball_coor[0] << ' ' << target_childball_coor[1] << endl;

                    int wrong_detect = 0;
                    for (int h = 0; h < n; h++)//所有子球
                    {
                        double PL_distance = dis(rel_cueball_target[0], rel_cueball_target[1], real_cueballcoor[0], real_cueballcoor[1], real_childball_coor[h][0], real_childball_coor[h][1]);


                        if (((abs(PL_distance) < boundval2) && mag(rel_child_targetchild[h][0], rel_child_targetchild[h][1]) < mag(rel_cueball_target[0], rel_cueball_target[1])))
                        {
                            //第二層運算不通過 母球及子球間有干擾
                            cout << "not pass!!!" << endl;
                            wrong_detect++;
                        }
                       
                        double angle2 = abs(acos(COS_VAL(target_holecoor[0] - target_childball_coor[0], target_holecoor[1] - target_childball_coor[1], real_cueballcoor[0] - target_childball_coor[0], real_cueballcoor[1] - target_childball_coor[1])) * 180 / 3.1415926);
                        if (angle2 < 110 || mag(rel_cueball_target[0], rel_cueball_target[1]) < 25) wrong_detect++;//不可能打進的球
                    }
                    //cout << target_childball_coor[0] << ' ' << target_childball_coor[1] << endl;
                    cout << wrong_detect << endl;
                    if (wrong_detect == 0)
                    {
                        //第二層通過

                        final_childball_coor[0] = target_childball_coor[0];
                        final_childball_coor[1] = target_childball_coor[1];
                        final_hole_coor[0] = target_holecoor[0];
                        final_hole_coor[1] = target_holecoor[1];

                        childresult[resultidx][0] = target_childball_coor[0];
                        childresult[resultidx][1] = target_childball_coor[1];
                        holeresult[resultidx][0] = target_holecoor[0];
                        holeresult[resultidx][1] = target_holecoor[1];
                        resultidx++;
                    }

                }
            }
        }
    }
  
    std::cout << "可行球數目:" << resultidx << endl;

    if (resultidx > 0) {//對找到的球做總(母球-子球，子球-洞口)距離排序
        for (int i = 0; i < resultidx; i++) {
            for (int j = i + 1; j < resultidx; j++) {
                double mag1 = mag(holeresult[i][0] - childresult[i][0], holeresult[i][1] - childresult[i][1]) + mag(real_cueballcoor[0] - childresult[i][0], real_cueballcoor[1] - childresult[i][1]);
                double mag2 = mag(holeresult[j][0] - childresult[j][0], holeresult[j][1] - childresult[j][1]) + mag(real_cueballcoor[0] - childresult[j][0], real_cueballcoor[1] - childresult[j][1]);
                if (mag2 < mag1) {
                    double temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
                    temp1 = childresult[i][0];
                    temp2 = childresult[i][1];
                    temp3 = holeresult[i][0];
                    temp4 = holeresult[i][1];
                    childresult[i][0] = childresult[j][0];
                    childresult[i][1] = childresult[j][1];
                    holeresult[i][0] = holeresult[j][0];
                    holeresult[i][1] = holeresult[j][1];
                    childresult[j][0] = temp1;
                    childresult[j][1] = temp2;
                    holeresult[j][0] = temp3;
                    holeresult[j][1] = temp4;

                }

            }
        }
        target_childball_coor[0] = childresult[0][0];
        target_childball_coor[1] = childresult[0][1];
        target_holecoor[0] = holeresult[0][0];
        target_holecoor[1] = holeresult[0][1];
        final_hole_coor[0] = target_holecoor[0];
        final_hole_coor[1] = target_holecoor[1];
        final_childball_coor[0] = target_childball_coor[0];
        final_childball_coor[1] = target_childball_coor[1];
    }

    ///////////////////////////////////////////////////////////////////
    //以下為彈射檢測
    ///////////////////////////////////////////////////////////////////
    
    std::cout << "洞口無干擾球數目:" << selected_ball_counter << endl;

    vector<vector<double>> collide_coor_project(selected_ball_counter * 4, vector<double>(12, 0));
    //每個撞擊點 對每個邊做投影 一為投影點x 二為投影點y 三為母球與投影x向量 四為母球與投影y向量 5為母球至彈射點x向量 6為母球至彈射點y向量 7為撞擊點x 8為撞擊點y 9.10為目標子球x.y 11.12為目標洞口x.y
   
    int project_count = 0;
    double project_flip_ratio = 0;
    vector<double> flip_target_vec(2, 0);//母球至彈射點之方向向量
    vector<double> flip_goal_coor(2, 0);//撞擊點座標
    vector<double> flip_goal_hole_coor(2, 0);//目標洞口座標
    vector<double> flip_target_ball_coor(2, 0);//目標子球座標
    vector<double> flip_point_coor(2, 0);//彈射點座標

    int flip_detect = 0;//檢測是否彈射
    if (resultidx == 10)
    {
        for (int k = selected_ball_counter - 1; k >= 0; k--)//selected_ball_counter陣列中有所有候選子球 由距離最大的開始找 若找到更小的便變為更小的量值
        {
            for (int i = 0; i < n; i++)//子球
            {
                for (int j = 0; j < 6; j++)//洞口
                {
                    if ((real_childball_coor[i][0] - real_holecoor[j][0]) == selected_childball_hole[k][0] && real_childball_coor[i][1] - real_holecoor[j][1] == selected_childball_hole[k][1])
                    {
                        vector<double>rel_hole_ball_vec(2, 0);
                        vector<double>rel_cueball_target(2, 0);
                        
                        rel_hole_ball_vec[0] = real_childball_coor[i][0] - real_holecoor[j][0];
                        rel_hole_ball_vec[1] = real_childball_coor[i][1] - real_holecoor[j][1];
                        double rel_hole_ball_mag = mag(rel_hole_ball_vec[0], rel_hole_ball_vec[1]);

                        //找撞擊點
                        rel_cueball_target[0] = real_childball_coor[i][0] + (ball_diameter-5)*rel_hole_ball_vec[0]/rel_hole_ball_mag;
                        rel_cueball_target[1] = real_childball_coor[i][1] + (ball_diameter-5)*rel_hole_ball_vec[1]/rel_hole_ball_mag;
                        /////////////////////////////////////////////////////////////////////////////////
                        //開始計算
                        /////////////////////////////////////////////////////////////////////////////////

                        //對top投影
                        double rel_top = top - rel_cueball_target[0] - 13;//撞擊點球半徑;
                        double cue_rel_top = top - real_cueballcoor[0] - 13;
                        collide_coor_project[project_count][0] = rel_cueball_target[0] + rel_top + cue_rel_top;
                        collide_coor_project[project_count][1] = rel_cueball_target[1];
                        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - real_cueballcoor[0];
                        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - real_cueballcoor[1];
                        project_flip_ratio = cue_rel_top / collide_coor_project[project_count][2];
                        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
                        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
                        collide_coor_project[project_count][6] = rel_cueball_target[0];
                        collide_coor_project[project_count][7] = rel_cueball_target[1];
                        collide_coor_project[project_count][8] = real_childball_coor[i][0];
                        collide_coor_project[project_count][9] = real_childball_coor[i][1];
                        collide_coor_project[project_count][10] = real_holecoor[j][0];
                        collide_coor_project[project_count][11] = real_holecoor[j][1];
                        if (real_cueballcoor[0] < 152) {//母球在下半部 不對遠邊(即頂部)做彈射
                            collide_coor_project[project_count][4] = 10000;
                            collide_coor_project[project_count][5] = 10000;
                        }
                        project_count++;

                        //對left投影
                        double rel_left = left - rel_cueball_target[1] + 13;
                        double cue_rel_left = left - real_cueballcoor[1] + 13;
                        collide_coor_project[project_count][0] = rel_cueball_target[0];
                        collide_coor_project[project_count][1] = rel_cueball_target[1]+rel_left+ cue_rel_left;
                        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - real_cueballcoor[0];
                        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - real_cueballcoor[1];
                        project_flip_ratio = cue_rel_left / collide_coor_project[project_count][3];
                        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
                        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
                        collide_coor_project[project_count][6] = rel_cueball_target[0];
                        collide_coor_project[project_count][7] = rel_cueball_target[1];
                        collide_coor_project[project_count][8] = real_childball_coor[i][0];
                        collide_coor_project[project_count][9] = real_childball_coor[i][1];
                        collide_coor_project[project_count][10] = real_holecoor[j][0];
                        collide_coor_project[project_count][11] = real_holecoor[j][1];
                        project_count++;

                        //對bottom投影
                        double rel_bottom = bottom - rel_cueball_target[0] + 13;
                        double cue_rel_bottom = bottom - real_cueballcoor[0] + 13;
                        collide_coor_project[project_count][0] = rel_cueball_target[0] + rel_bottom + cue_rel_bottom;
                        collide_coor_project[project_count][1] = rel_cueball_target[1];
                        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - real_cueballcoor[0];
                        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - real_cueballcoor[1];
                        project_flip_ratio = cue_rel_bottom / collide_coor_project[project_count][2];
                        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
                        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
                        collide_coor_project[project_count][6] = rel_cueball_target[0];
                        collide_coor_project[project_count][7] = rel_cueball_target[1];
                        collide_coor_project[project_count][8] = real_childball_coor[i][0];
                        collide_coor_project[project_count][9] = real_childball_coor[i][1];
                        collide_coor_project[project_count][10] = real_holecoor[j][0];
                        collide_coor_project[project_count][11] = real_holecoor[j][1];
                        if (real_cueballcoor[0] > 152) {//母球在上半部 不對遠邊(即底部)做彈射
                            collide_coor_project[project_count][4] = 10000;
                            collide_coor_project[project_count][5] = 10000;
                        }
                        project_count++;

                        //對right投影
                        double rel_right = right - rel_cueball_target[1] - 13;
                        double cue_rel_right = right - real_cueballcoor[1] - 13;
                        collide_coor_project[project_count][0] = rel_cueball_target[0];
                        collide_coor_project[project_count][1] = rel_cueball_target[1] + rel_right+ cue_rel_right;
                        collide_coor_project[project_count][2] = collide_coor_project[project_count][0] - real_cueballcoor[0];
                        collide_coor_project[project_count][3] = collide_coor_project[project_count][1] - real_cueballcoor[1];
                        project_flip_ratio = cue_rel_right / collide_coor_project[project_count][3];
                        collide_coor_project[project_count][4] = collide_coor_project[project_count][2] * project_flip_ratio;
                        collide_coor_project[project_count][5] = collide_coor_project[project_count][3] * project_flip_ratio;
                        collide_coor_project[project_count][6] = rel_cueball_target[0];
                        collide_coor_project[project_count][7] = rel_cueball_target[1];
                        collide_coor_project[project_count][8] = real_childball_coor[i][0];
                        collide_coor_project[project_count][9] = real_childball_coor[i][1];
                        collide_coor_project[project_count][10] = real_holecoor[j][0];
                        collide_coor_project[project_count][11] = real_holecoor[j][1];
                        project_count++;
                    }
                }
            }
        }
        vector<vector<double>> collide_coor_project_vec_detect(selected_ball_counter * 4, vector<double>(8, 0));
        //1.2為母球至彈射點x.y向量 3.4彈射點x.y座標 5.6為彈射點至撞擊點x.y向量 7.8為檢測彈射前、彈射後可行性 若12為10000則為對遠邊做彈射

        for (int i = 0; i < selected_ball_counter * 4; i++) {
            collide_coor_project_vec_detect[i][0] = collide_coor_project[i][4];
            collide_coor_project_vec_detect[i][1] = collide_coor_project[i][5];
            collide_coor_project_vec_detect[i][2] = real_cueballcoor[0] + collide_coor_project[i][4];
            collide_coor_project_vec_detect[i][3] = real_cueballcoor[1] + collide_coor_project[i][5];
            collide_coor_project_vec_detect[i][4] = collide_coor_project[i][6] - collide_coor_project_vec_detect[i][2];
            collide_coor_project_vec_detect[i][5] = collide_coor_project[i][7] - collide_coor_project_vec_detect[i][3];
            if (collide_coor_project_vec_detect[i][0] > 5000) {
                collide_coor_project_vec_detect[i][6] = 10000;

            }
            double target_child_x = 0;
            double target_child_y = 0;
            target_child_x = collide_coor_project[i][8];
            target_child_y = collide_coor_project[i][9];
            //std::cout << "彈射點座標" << collide_coor_project_vec_detect[i][2] << " " << collide_coor_project_vec_detect[i][2] << endl;
            //std::cout << "撞擊點座標" << collide_coor_project[i][6] << " " << collide_coor_project[i][7] << endl;
            //std::cout << "彈射點至撞擊點向量" << collide_coor_project_vec_detect[i][4] << " " << collide_coor_project_vec_detect[i][5] << endl;

            for (int k = 0; k < n; k++) {
                double pl_distance = dis(collide_coor_project_vec_detect[i][0], collide_coor_project_vec_detect[i][1], real_cueballcoor[0], real_cueballcoor[1], real_childball_coor[k][0], real_childball_coor[k][1]);

                //彈射前干擾 目標子球可以先不替除
                if (((abs(pl_distance) < boundval2)&&mag(collide_coor_project_vec_detect[i][2]- real_childball_coor[k][0], collide_coor_project_vec_detect[i][3] - real_childball_coor[k][1]) < mag(collide_coor_project_vec_detect[i][0], collide_coor_project_vec_detect[i][1])))
                {
                    collide_coor_project_vec_detect[i][6] = 10000;
                    std::cout << "彈射前干擾" << endl;
                }

                if (collide_coor_project_vec_detect[i][6] < 1000) 
                {//彈射後干擾
                    double pl_distance = dis(collide_coor_project_vec_detect[i][4], collide_coor_project_vec_detect[i][5], collide_coor_project_vec_detect[i][2], collide_coor_project_vec_detect[i][3], real_childball_coor[k][0], real_childball_coor[k][1]);
                    //collide_coor_project[i][8]為目標子球座標x 9為y
                    double target_identify_x = collide_coor_project[i][8] - real_childball_coor[k][0];
                    double target_identify_y = collide_coor_project[i][9] - real_childball_coor[k][1];
                    double otherchild_flip_dis_x = collide_coor_project_vec_detect[i][2] - real_childball_coor[k][0];
                    double otherchild_flip_dis_y = collide_coor_project_vec_detect[i][3] - real_childball_coor[k][1];

                    if (((abs(pl_distance) < boundval2) && target_identify_x > 2 && target_identify_y > 2 && mag(otherchild_flip_dis_x, otherchild_flip_dis_y) < mag(collide_coor_project_vec_detect[i][4], collide_coor_project_vec_detect[i][5]))) {
                        //其他子球至撞擊點向量與彈射向量之夾角
                        double angle2 = abs(acos(COS_VAL(otherchild_flip_dis_x, otherchild_flip_dis_y, collide_coor_project_vec_detect[i][4], collide_coor_project_vec_detect[i][5])) * 180 / 3.1415926);
                        if (angle2>90) {
                            collide_coor_project_vec_detect[i][7] = 10000;
                            std::cout << "彈射後干擾" << endl;
                        }
                    }
                }
                //本身干擾
                double angle_itself = abs(acos(COS_VAL(collide_coor_project_vec_detect[i][4], collide_coor_project_vec_detect[i][5], target_child_x- collide_coor_project[i][6], target_child_y- collide_coor_project[i][7])) * 180 / 3.1415926);
                if (angle_itself > 70) {
                    collide_coor_project_vec_detect[i][7] = 10000;
                    std::cout << "彈射後干擾" << endl;
                }
            }
        }
        vector<vector<double>> available_flip(selected_ball_counter * 4, vector<double>(15, 0));
        //1.2母球x.y 3.4母球至彈射x.y 5.6彈射x.y 7.8彈射至撞擊點x.y 9.10撞擊點x.y 11.12目標子球 13.14目標洞口 15總距離
        int count = 0;

        for (int i = 0; i < selected_ball_counter * 4; i++) {
            if (collide_coor_project_vec_detect[i][7] < 5000 && collide_coor_project_vec_detect[i][8] < 5000) {
                available_flip[count][0] = real_cueballcoor[0];
                available_flip[count][1] = real_cueballcoor[1];
                available_flip[count][2] = collide_coor_project_vec_detect[i][0];
                available_flip[count][3] = collide_coor_project_vec_detect[i][1];
                available_flip[count][4] = collide_coor_project_vec_detect[i][2];
                available_flip[count][5] = collide_coor_project_vec_detect[i][3];
                available_flip[count][6] = collide_coor_project_vec_detect[i][4];
                available_flip[count][7] = collide_coor_project_vec_detect[i][5];
                available_flip[count][8] = available_flip[count][4] + available_flip[count][6];
                available_flip[count][9] = available_flip[count][5] + available_flip[count][7];
                available_flip[count][10] = collide_coor_project[i][8];
                available_flip[count][11] = collide_coor_project[i][9];
                available_flip[count][12] = collide_coor_project[i][10];
                available_flip[count][13] = collide_coor_project[i][11];
                available_flip[count][14] = mag(available_flip[count][2], available_flip[count][3]) + mag(available_flip[count][6], available_flip[count][7]);
                count++;
            }
        }
        double mini = 10000;
        int detect = 0;
        std::cout << "可行球: " << count << endl;
        for (int i = 0; i < count; i++) {
            if (available_flip[i][14] < mini) {
                flip_target_vec[0] = available_flip[i][2] / mag(available_flip[i][2], available_flip[i][3]);
                flip_target_vec[1] = available_flip[i][3] / mag(available_flip[i][2], available_flip[i][3]);
                mini = available_flip[i][14];
                detect = i;
            }
        }
        
        ofstream flip_once_point;
        flip_once_point.open("c:/users/wang8/source/repos/c++repeat/c++repeat/flip_once_point.csv", ios::out);
        //1.2母球x.y 3.4母球至彈射x.y 5.6彈射x.y 7.8彈射至撞擊點x.y 9.10撞擊點x.y 11.12目標子球 13.14目標洞口 15總距離
        flip_once_point << available_flip[detect][0] << "," << available_flip[detect][1] << endl;//1.2母球x.y
        flip_once_point << available_flip[detect][2] << "," << available_flip[detect][3] << endl;//3.4母球至彈射x.y
        flip_once_point << available_flip[detect][4] << "," << available_flip[detect][5] << endl;//5.6彈射x.y
        flip_once_point << available_flip[detect][6] << "," << available_flip[detect][7] << endl;//7.8彈射至撞擊點x.y
        flip_once_point << available_flip[detect][8] << "," << available_flip[detect][9] << endl;//9.10撞擊點x.y
        flip_once_point << available_flip[detect][10] << "," << available_flip[detect][11] << endl;//11.12目標子球
        flip_once_point << available_flip[detect][12] << "," << available_flip[detect][13] << endl;//13.14目標洞口
        //打擊點
        flip_once_point << available_flip[detect][0]- (ball_diameter + cali ) * flip_target_vec[0] << "," << available_flip[detect][1]- (ball_diameter + cali) * flip_target_vec[1] << endl;
        flip_once_point.close();

        flip_goal_coor[0] = available_flip[detect][8];//撞擊點座標
        flip_goal_coor[1] = available_flip[detect][9];//撞擊點座標
        flip_goal_hole_coor[0]= available_flip[detect][12];//目標洞口座標
        flip_goal_hole_coor[1] = available_flip[detect][13];
        flip_target_ball_coor[0]= available_flip[detect][10];//目標子球座標
        flip_target_ball_coor[1] = available_flip[detect][11];
        flip_point_coor[0]= available_flip[detect][4];//彈射點座標
        flip_point_coor[1] = available_flip[detect][5];

        final_hole_coor[0] = available_flip[detect][12];
        final_hole_coor[1] = available_flip[detect][13];
        final_childball_coor[0] = available_flip[detect][10];
        final_childball_coor[1] = available_flip[detect][11];
        flip_detect = 1;

        std::cout << "彈射檢測: " << flip_detect << endl;
        std::cout << "母球: " << available_flip[detect][0] << " " << available_flip[detect][1] << endl;
        std::cout << "彈射點: " << available_flip[detect][4] << " " << available_flip[detect][5] << endl;
        std::cout << "撞擊點: " << available_flip[detect][8] << " " << available_flip[detect][9] << endl;

    }

    if (resultidx == 0 && flip_detect==0) {
        final_childball_coor[0] = real_childball_coor[0][0];
        final_childball_coor[1] = real_childball_coor[0][1];

        final_hole_coor[0] = real_childball_coor[0][0] * 2 - real_cueballcoor[0];
        final_hole_coor[1] = real_childball_coor[0][1] * 2 - real_cueballcoor[1];
    }

    double x_cueball_coor = 0, y_cueball_coor = 0, z_cueball_coor = 0;
    double x_goal_coor = 0, y_goal_coor = 0, z_goal_coor = 0;//
    double hole[3] = { 0 }, hited_ball[3] = { 0 }, hole_rel_hitedball[3] = { 0 };

    hole[0] = final_hole_coor[0];
    hole[1] = final_hole_coor[1];
    hole[2] = z;
    hited_ball[0] = final_childball_coor[0];
    hited_ball[1] = final_childball_coor[1];
    hited_ball[2] = z;
    x_cueball_coor = real_cueballcoor[0];
    y_cueball_coor = real_cueballcoor[1];
    z_cueball_coor = z;

    for (size_t i = 0; i < 3; i++)
    {
        hole_rel_hitedball[i] = hole[i] - hited_ball[i];//洞口-子球之向量
    }
    double mag_hole_rel_hitedball = 0;
    for (size_t i = 0; i < 2; i++)
    {
        mag_hole_rel_hitedball += pow(hole_rel_hitedball[i], 2);//洞口-子球之向量大小平方
    }
    mag_hole_rel_hitedball = sqrt(mag_hole_rel_hitedball);//洞口-子球之向量大小
    for (size_t i = 0; i < 3; i++)
    {
        hole_rel_hitedball[i] = hole_rel_hitedball[i] / mag_hole_rel_hitedball;//換為單位向量
    }
    x_goal_coor = hited_ball[0] - (ball_diameter-3) * hole_rel_hitedball[0];
    y_goal_coor = hited_ball[1] - (ball_diameter-3) * hole_rel_hitedball[1];
    z_goal_coor = hited_ball[2] - (ball_diameter-3) * hole_rel_hitedball[2];//母球終點位置在目標子球格一顆球之位置

    if (flip_detect == 1) {
        x_goal_coor = flip_goal_coor[0];
        y_goal_coor = flip_goal_coor[1];
    }

    double pos_cueball[6] = { 0 };
    //double a[6] = { 500,400,200,90,90, 0 };

    double rel_x = x_goal_coor - x_cueball_coor;//母球至目標點向量x分量
    double rel_y = y_goal_coor - y_cueball_coor;
    double rel_z = z_goal_coor - z_cueball_coor;
    double rel_dis = sqrt(pow(rel_x, 2) + pow(rel_y, 2));
    //double TCP_move_x = rel_x / rel_dis;
    //double TCP_move_y = rel_y / rel_dis;
    double vector_x = rel_x / rel_dis;//母球至目標點單位向量x分量
    double vector_y = rel_y / rel_dis;//母球至目標點單位向量y分量
    
    if (flip_detect == 1) {//若為彈射 打擊方向須修正
        vector_x = flip_target_vec[0];
        vector_y = flip_target_vec[1];
    }
    
    double cue_target_dis = 0;
    int theta_cali = 0;
    cue_target_dis = mag(rel_x, rel_y)+mag(final_hole_coor[0]- x_goal_coor, final_hole_coor[1] - y_goal_coor);

    if (flip_detect == 1) {//若為彈射 打擊距離須修正
        cue_target_dis = mag(flip_point_coor[0] - x_cueball_coor, flip_point_coor[1] - y_cueball_coor) + mag(x_goal_coor - flip_point_coor[0], y_goal_coor - flip_point_coor[1])+ mag(final_hole_coor[0] - x_goal_coor, final_hole_coor[1] - y_goal_coor);
    }

    std::cout << "距離" << cue_target_dis << endl;
    std::cout << "原始打擊點:" << x_cueball_coor - (ball_diameter + cali + theta_cali) * vector_x<<" "<< y_cueball_coor - (ball_diameter + cali + theta_cali) * vector_y << endl;
    
    if (cue_target_dis < 200 and cue_target_dis >=175) {//距離補正
        std::cout << "近" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis <= 175 and cue_target_dis >= 150) {
        std::cout << "比近再近一點" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis <= 150 and cue_target_dis >= 100) {
        std::cout << "很近" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis <= 100) {
        std::cout << "超近" << endl;
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis >= 200 && cue_target_dis < 250) {
        std::cout << "中等" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    
    else if (cue_target_dis >= 250 && cue_target_dis < 350) {
        std::cout << "中等偏遠" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else if (cue_target_dis >= 350 && cue_target_dis < 450) {
        std::cout << "中遠" << endl;
        set_digital_output(device_id, 15, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, false);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, false);//設定輸出 似乎是打擊
    }
    else {
        std::cout << "遠" << endl;
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊
    }
    std::cout << "調整打擊點:" << x_cueball_coor - (ball_diameter + cali + theta_cali) * vector_x << " " << y_cueball_coor - (ball_diameter + cali + theta_cali) * vector_y << endl;
    double vector_HIT_PATH[3] = { vector_x, vector_y, 0 };
    double vector_TCP[3] = { 0,-1,0 };
    double inner_product = 0;
    double sum = 0;
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

    double origin_point[6] = { 90,0,0,0,-90,0 };

    ofstream pythonplot;
    pythonplot.open("C:/Users/wang8/source/repos/C++repeat/C++repeat/circle.csv", ios::out);
    //pythonplot.open("C:/Users/wang8/source/repos/cflip/cflip/pythonplot.xlsx", ios::out);
    pythonplot << top+20 << "," << right << endl;
    pythonplot << top << "," << left + 10 << endl;
    pythonplot << middle << "," << right << endl;
    pythonplot << middle << "," << left << endl;
    pythonplot << bottom + 5 << "," << right - 10 << endl;
    pythonplot << bottom << "," << left << endl;
    pythonplot << real_cueballcoor[0] << "," << real_cueballcoor[1] << endl;
    pythonplot << hited_ball[0] << "," << hited_ball[1] << endl;
    pythonplot << hole[0] << "," << hole[1] << endl;
    pythonplot.close();

    double final_coor_x = 0;
    final_coor_x = x_cueball_coor - (ball_diameter + cali + theta_cali) * vector_x;
    double final_coor_y = 0;
    final_coor_y = y_cueball_coor - (ball_diameter + cali + theta_cali) * vector_y;
    std::cout<<"母球:" << x_cueball_coor << "  " << y_cueball_coor << endl;
    std::cout << "目標點:" << x_goal_coor << " " << y_goal_coor << endl;
    std::cout << "目標球:" << hited_ball[0] << " " << hited_ball[1] << endl;
    std::cout << "目標洞口:" << hole[0] << " " << hole[1] << endl;
    if (final_coor_x >= (455+hitter_a) ) {
        final_coor_x = 455+ hitter_a;
    }
    else if(final_coor_x<=-158- hitter_a){
        final_coor_x = -158- hitter_a;
    }
    if (final_coor_y >= 195+ hitter_a) {
        final_coor_y = 195+ hitter_a;
    }
    else if (final_coor_y <= -95- hitter_a) {
        final_coor_y = -95- hitter_a;
    }

    double origin_point_inner[6] = { 90,0,15,0,-105,0 };
    

    if (vector_x > 0)
    {
        //pos_cueball = { x_cueball_coor, y_cueball_coor, z_cueball_coor ,0,0,-theta};
        *pos_cueball = final_coor_x;
        *(pos_cueball + 1) = final_coor_y;
        *(pos_cueball + 2) = z;
        *(pos_cueball + 3) = 0;
        *(pos_cueball + 4) = 0;
        *(pos_cueball + 5) = -90+theta;
        //lin_pos(device_id, 0, 0, pos_cueball);
        //lin_pos(device_id, 0,0, pos_cueball);
        ptp_pos(device_id, 0, pos_cueball);
        wait(device_id);
        *(pos_cueball + 2) = z_correct;
        wait(device_id);
        lin_pos(device_id, 0, 0, pos_cueball);
        ptp_pos(device_id, 0, pos_cueball);
        wait(device_id);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        Sleep(500);
        set_digital_output(device_id, 16, true);
        Sleep(500);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        wait(device_id);
        //ptp_axis(device_id,1, origin_point);
        //jog_home(device_id);
        //Sleep(2000);
        ptp_axis(device_id, 0, origin_point_inner);
        wait(device_id);
        std::cout << "?" << std::endl;

    }
    else
    {
        //pos_cueball = { x_cueball_coor, y_cueball_coor, z_cueball_coor ,0,0,theta };
        *pos_cueball = final_coor_x;
        *(pos_cueball + 1) = final_coor_y;
        *(pos_cueball + 2) = z;
        *(pos_cueball + 3) = 0;
        *(pos_cueball + 4) = 0;
        *(pos_cueball + 5) = -90-theta;
        //lin_pos(device_id, 0,0, pos_cueball);
        ptp_pos(device_id, 0, pos_cueball);
        wait(device_id);
        *(pos_cueball + 2) = z_correct;
        //lin_pos(device_id, 0, 0, pos_cueball);
        wait(device_id);
        ptp_pos(device_id, 0, pos_cueball);
        wait(device_id);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        Sleep(500);
        set_digital_output(device_id, 16, true);
        Sleep(500);
        set_digital_output(device_id, 16, false);//設定輸出 似乎是打擊
        wait(device_id);

        //ptp_axis(device_id,1, origin_point);
        //jog_home(device_id);
        //Sleep(2000);
        ptp_axis(device_id,0,origin_point_inner);
        wait(device_id);
        std::cout << "!" << std::endl;
    }
    for (int i = 0; i < 6; i++)
    {
        cout << pos_cueball[i] << ' ';
    }
    cout << endl;
    std::cout << "結束" << endl;
    set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
    set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊
    std::cout << "程式:" << 3 << endl;
}
int main() {
   
    
    char sdk_ver[50];
    char hrss_ver[50];
    get_hrsdk_version(sdk_ver);
    std::cout << "SDK version: " << sdk_ver << std::endl;
    HROBOT device_id = open_connection("169.254.148.16", 1, callBack);//169.254.225.26原 172.20.10.3測1 192.168.100.45HRSS
    if (device_id >= 0) {
        set_digital_output(device_id, 15, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 14, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 13, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 12, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 11, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 10, true);//設定輸出 似乎是打擊
        set_digital_output(device_id, 9, true);//設定輸出 似乎是打擊

        double origin_point[6] = { 90,0,15,0,-105,0 };
        set_home_point(device_id, origin_point);

        set_operation_mode(device_id,1);
        int mode = 0;
        mode = get_hrss_mode(device_id);
        std::cout << "mode: " << mode << endl;
        set_override_ratio(device_id, 60);
        set_acc_dec_ratio(device_id, 45);
        //set_lin_speed(device_id, 1500);
        double speed = 0;

        std::cout << "HRSS version: " << hrss_ver << std::endl;
        std::cout << "connect successful." << std::endl;
        int hit_count = 0;
        //LinExample(device_id);
        FILE* fp1 = fopen("D:/hiwin/vscode_python/circle_detect_640.py", "r");
        Py_Initialize();
        PyRun_AnyFile(fp1, "D:/hiwin/vscode_python/circle_detect_640.py");
        std::cout << "fuck" << endl;
        Py_Finalize();
        fclose(fp1);
        to_enter_point(device_id);

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


double INNER_PRODUCT(double a, double b, double c, double d)
{
    return a * c + b * d;
}
double mag(double a, double b)
{
    return sqrt(a * a + b * b);
}
double COS_VAL(double a, double b, double c, double d)
{
    return (INNER_PRODUCT(a, b, c, d) / (mag(a, b) * mag(c, d)));
}
double dis(double vec_x, double vec_y, double pass_x, double pass_y, double x0, double y0)
{
    double c = vec_y * pass_x - vec_x * pass_y;
    double distance = (vec_y * x0 - vec_x * y0 - c) / sqrt(vec_x * vec_x + vec_y * vec_y);
    return distance;
}
//////////////////////////////////////////////////////////////////////





