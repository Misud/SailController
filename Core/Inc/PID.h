/*
    PID
    用于管理PID相关代码
*/

#ifndef __PID_H
#define __PID_H

#include <stdint.h>

// PID 参数结构体
typedef struct{
    float Kp;
    float Ki;
    float Kd;

    float target;           // 目标角度
    float actual;           // 实际角度 MPU6050获取

    float err;              // 当前误差
    float last_err;         // 上次误差
    float integral;         // 累计误差

    float out_max;          // 输出限幅
    float integral_max;     // 积分限幅

    float Output;           // PID输出

    float K_ff;             // 重力前馈系数
    uint8_t enable_ff;      // 使能前馈
}PID_t;

// PID初始化
void PID_Init(PID_t *pid, float kp, float ki, float kd, float out_max);

// PID输出
float PID_Compute(PID_t *pid, float target, float actual, float dt);

#endif