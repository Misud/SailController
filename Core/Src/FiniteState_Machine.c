/*
    Finite_State_Machine
    ״̬������
    ����״̬��*1 ��״̬��*5
*/

#include "FiniteState_Machine.h"
#include "Motor.h"
#include "PID.h"
#include "KeyScan.h"
#include "Light_Sound.h"
#include <math.h>

// main.c 中的全局变量
extern float target_angle;
extern float current_angle;
extern volatile KeyValue_t key_from_isr;
extern volatile uint32_t sys_tick_ms;

// 初始角度
#define INIT_ANGLE      90.0f   // 初始角度
#define ANGLE_TOLERANCE 2.0f    // 误差容限

SystemState_t current_state = STATE_IDLE;   

// 子状态机变量
static uint8_t      sub_state   = 0;
static uint32_t     start_time  = 0;
static uint8_t      step_angle  = 45;
static uint8_t      swing_count = 0;

// 状态机初始化
void FSM_Init(void)
{
    current_state = STATE_IDLE;
    sub_state = 0;
    target_angle = INIT_ANGLE;
}

// 状态机更新
void FSM_Update(void)
{
    // 按键切换顶层状态
    if(key_from_isr != Key_None)
    {
        if(key_from_isr == Key_1)   { current_state = STATE_CMD1; sub_state = 0; }
        if(key_from_isr == Key_2)   { current_state = STATE_CMD2; sub_state = 0; step_angle = 45; }
        if(key_from_isr == Key_3)   { current_state = STATE_CMD3; sub_state = 0; swing_count = 0; }
        // KEY4暂不出列

        key_from_isr = Key_None;    // 重置按键
    }

    // 执行子状态机
    switch(current_state)
    {
        case STATE_IDLE: // 初始化
            target_angle = INIT_ANGLE;
        break;

        case STATE_CMD1: // 到达预设角度
            switch(sub_state)
            {
                case 0: // 转至设定角度
                    target_angle = 60.0f;   // 预设值
                    if( fabsf(target_angle - current_angle) < ANGLE_TOLERANCE )
                    {
                        start_time = sys_tick_ms;   // 记录实践戳
                        sub_state = 1;
                    }
                    break;

                case 1: // 稳定5s
                    if( fabsf(target_angle - current_angle) > ANGLE_TOLERANCE )
                    {
                        sub_state = 0;  // 有偏移，重调
                    }
                    else if(sys_tick_ms - start_time >= 5000)
                    {
                        sub_state = 2;
                    }
                    break;

                case 2: // 声光提示+复位
                    Light_Sound_Start(800);
                    target_angle = INIT_ANGLE;
                    sub_state = 3;
                    break;

                case 3: // 恢复至初始化状态
                    if((fabsf(target_angle - current_angle) < ANGLE_TOLERANCE) && Light_Sound_Done())
                    {
                        current_state = STATE_IDLE;
                    }
                    break;
            }
        break;

        case STATE_CMD2: // 角度步进
            switch(sub_state)
            {
                case 0: // 设置目标
                    target_angle = (float)step_angle;
                    sub_state = 1;
                    break;

                case 1: // 转至目标角度
                    if(fabsf(target_angle - current_angle) < ANGLE_TOLERANCE)
                    {
                        start_time = sys_tick_ms;
                        sub_state = 2;
                    }
                    break;

                case 2: // 维持5s
                    if(fabsf(target_angle - current_angle) > ANGLE_TOLERANCE)
                    {
                        sub_state = 1;
                    }
                    else if(sys_tick_ms - start_time >= 5000)
                    {
                        Light_Sound_Start(800);
                        sub_state = 3;
                    }
                    break;

                case 3: // 灯光结束后步进
                    if( fabsf(target_angle - current_angle) < ANGLE_TOLERANCE && Light_Sound_Done() )
                    {
                        step_angle += 15;
                        if(step_angle > 135)
                        {
                            current_state = STATE_IDLE; // 遍历完成
                        }
                        else
                        {
                            sub_state = 0;
                        }
                    }
                    break;
                    
            }
            break;

        case STATE_CMD3: // 往复运动
            switch(sub_state)
            {
                case 0: // 设置A角度
                    target_angle = 60.0f;   // 预设
                    sub_state = 1;
                    break;

                case 1: // 等待到达A角度
                    if( fabsf(target_angle - current_angle) < ANGLE_TOLERANCE )
                    {
                        Light_Sound_Start(500);
                        sub_state = 2;
                    }
                    break;

                case 2: // 设置B角度
                    if(Light_Sound_Done())
                    {
                        target_angle = 120.0f;  // 预设
                        sub_state = 3;
                    }
                    break;

                case 3: // 等待到达B角度
                    if( fabsf(target_angle - current_angle) < ANGLE_TOLERANCE )
                    {
                        Light_Sound_Start(500);
                        sub_state = 4;
                    }
                    break;

                case 4: // 统计次数
                    if(Light_Sound_Done())
                    {
                        swing_count++;
                        if(swing_count >= 5) // 预设
                        {
                            current_state = STATE_IDLE;
                        }
                        else{
                            sub_state = 0;
                        }
                    }
                    break;
                    
            }
            break;
    }
}