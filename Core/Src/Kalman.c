/*
    Kalman Filter
    管理卡尔曼滤波器
    卡尔曼滤波器用于对MPU6050的角度信息整形
*/

#include "Kalman.h"


/*==========卡尔曼滤波初始化==========*/
void Kalman_Init(Kalman_t *kalman)
{
    kalman->angle   = 0.0f;
    kalman->bias    = 0.0f;
    kalman->P[0][0] = 0.0f;
    kalman->P[0][1] = 0.0f;
    kalman->P[1][0] = 0.0f;
    kalman->P[1][1] = 0.0f;

    // Q_angle 越大 越信任加速度计
    // R_Measure 越大 越信任陀螺仪
    kalman->Q_angle = 0.10f;
    kalman->Q_bias  = 0.003f;
    kalman->R_measure = 0.10f;
}


/*==========Kalman 滤波核心==========*/
// 在 TIM1 1kHz 中断中回调
float Kalman_Update(Kalman_t *kalman, float newAngle, float newRate, float dt)
{
    // 1. 利用 积分 进行预测
    kalman->angle += dt * (newRate - kalman->bias);

    // 误差协方差矩阵预测
    kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;

    // 2. 更新，加权平均
    // 计算卡尔曼增益
    float S = kalman->P[0][0] + kalman->R_measure;
    float K[2];
    K[0] = kalman->P[0][0] / S;
    K[1] = kalman->P[1][0] / S;

    // 计算残差（测量值 - 预测值）
    float y = newAngle - kalman->angle;

    // 更新状态（修正角度和零偏）
    kalman->angle += K[0] * y;
    kalman->bias  += K[1] * y;

    // 更新误差协方差
    float P00_temp = kalman->P[0][0];
    float P01_temp = kalman->P[0][1];

    kalman->P[0][0] -= K[0] * P00_temp;
    kalman->P[0][1] -= K[0] * P01_temp;
    kalman->P[1][0] -= K[1] * P00_temp;
    kalman->P[1][1] -= K[1] * P01_temp;

    return kalman->angle;
}