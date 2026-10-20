/*
    Motor
    用于管理电机和PWM
*/
#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

// 左侧为1号电机u1

// 初始化电机PWM
void Motor_Init(void);

// 设置电机站占空比 0%~100%
void Motor_SetPWM(float u1, float u2);


#endif