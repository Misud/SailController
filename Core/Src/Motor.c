/*
    Motor
    用于管理电机和PWM
*/

#include "Motor.h"
#include "tim.h"

#define PWM_MAX_ARR 3599.0f



// 设置两个电机的PWM
void Motor_SetPWM(float u1, float u2)
{
    // 限幅
    if(u1 > 100.0f) u1 = 100.0f;
    if(u1 < 0.0f)   u1 = 0.0f;

    if(u2 > 100.0f) u2 = 100.0f;
    if(u2 < 0.0f)   u2 = 0.0f;

    // 将占空比（0~100）分配到寄存器（0~3599）
    uint16_t ccr1 = (uint16_t)((u1 / 100.0f) * PWM_MAX_ARR); 
    uint16_t ccr2 = (uint16_t)((u2 / 100.0f) * PWM_MAX_ARR);

    // 更新寄存器
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr2);
}


// 初始化两个电机
void Motor_Init(void)
{
    // 开启TIM2 两个通道PWM输出
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    // 初始占空比 0
    Motor_SetPWM(0.0f, 0.0f);
}