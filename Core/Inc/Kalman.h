/*
    Kalman Filter
    管理卡尔曼滤波器
    卡尔曼滤波器用于对MPU6050的角度信息整形
*/

#ifndef __KALMAN_H
#define __KALMAN_H

// 一维卡尔曼滤波结构体
typedef struct{
    float angle;    // 滤波后的角度
    float bias;     // 陀螺仪零偏
    float P[2][2];  // 误差协方差矩阵
    float Q_angle;  // 过程噪声协方差（角度）
    float Q_bias;   // 过程噪声协方差（零偏）
    float R_measure;// 测量噪声协方差（加速度计）
}Kalman_t;


// 初始化
void Kalman_Init(Kalman_t *kalman);

// 一维卡尔曼滤波
float Kalman_Update(Kalman_t *kalman, float newAngle, float newRate, float dt);


#endif