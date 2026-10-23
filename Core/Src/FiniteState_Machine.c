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

#include "ringbuf.h"
#include "usbd_cdc_if.h"
#include "tim.h"
#include "OLED.h"
#include "frame.h"
#include "usart.h"

// main.c 中的全局变量
extern float target_angle;
extern float current_angle;
extern volatile KeyValue_t key_from_isr;
extern volatile uint32_t sys_tick_ms;
extern volatile uint8_t is_speed_mode;
extern volatile float omega_cmd;

extern float test_angle;
extern float test_omega;
extern PID_t pid_angle;    // 外环PID
extern PID_t pid_gyro;     // 内环PID

// 初始角度
#define INIT_ANGLE      90.0f   // 初始角度
#define ANGLE_TOLERANCE 2.0f    // 误差容限

SystemState_t current_state = STATE_IDLE;   

//winly:环形缓冲区
ringbuf_t ringbuffer;
//解析数据帧
uint8_t frame[64];
uint32_t frame_len;//找到的一帧的帧长度
//帧描述符
frame_desc_t desc = {
    .head = (const unsigned char*)"@@",
    .head_len = 2,
    .tail = (const unsigned char*)"##",
    .tail_len = 2,
    .len_size = 1,
    .has_crc = 0,
};
uint8_t ack[] = "ook";

// 子状态机变量
static uint8_t      sub_state   = 0;
static uint32_t     start_time  = 0;
static float        M1_angle    = 0;
static uint8_t      step_angle  = 45;
static float        M3_angleA   = 0;
static float        M3_angleB   = 0;
static uint8_t      swing_count = 0;
static float        M4_angleA   = 0;
static float        M4_angleB   = 0;


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
        if(key_from_isr == Key_4)   { current_state = STATE_CMD4; sub_state = 0; }

        key_from_isr = Key_None;    // 重置按键
    }
    
    //上位机切换顶层状态
    if(ringbuf_frame_find(&ringbuffer,&desc,frame,&frame_len)){
        //usb调参
        if(frame[3] == 'o'&&frame[4] == 'p') pid_angle.Kp = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'o'&&frame[4] == 'i') pid_angle.Ki = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'o'&&frame[4] == 'd') pid_angle.Kd = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'i'&&frame[4] == 'p') pid_gyro.Kp = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'i'&&frame[4] == 'i') pid_gyro.Kp = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'i'&&frame[4] == 'd') pid_gyro.Kp = readpid(frame,5,6,7,8,9);CDC_Transmit_FS(ack,3);
        if(frame[3] == 't'&&frame[4] == 't') test_angle = readangle(frame,5,6,7);CDC_Transmit_FS(ack,3);
        if(frame[3] == 't'&&frame[4] == '0') test_omega = readangle(frame,5,6,7);CDC_Transmit_FS(ack,3);
        if(frame[3] == 'a'&&frame[4] == 'c'){
            CDC_Transmit_FS(ack,3);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
        }
        if(frame[3] == 's'&&frame[4] == 't'){
            CDC_Transmit_FS(ack,3);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
        }
            
        //usb指令解析
        if(frame[3] == 'M'&&frame[4] == '1'){
            current_state = STATE_CMD1;
            sub_state = 0;
            M1_angle = readangle(frame,6,7,8);
        }else if(frame[3] == 'M'&&frame[4] == '2'){
            current_state = STATE_CMD2; 
            sub_state = 0; 
            step_angle = 45;
        }else if(frame[3] == 'M'&&frame[4] == '3'){
            current_state = STATE_CMD3; 
            sub_state = 0; 
            M3_angleA = readangle(frame,6,7,8);
            M3_angleB = readangle(frame,10,11,12);
            swing_count = frame[14] - '0';
        }else if(frame[3] == 'M'&&frame[4] == '4'){
            current_state = STATE_CMD4; 
            sub_state = 0;
            M4_angleA = readangle(frame,6,7,8);
            M4_angleB = readangle(frame,10,11,12);
        }
        
    }

    // 执行子状态机
    switch(current_state)
    {
        case STATE_IDLE: // 初始化
//            target_angle = INIT_ANGLE;
            target_angle = test_angle;//调试pid使用
        break;

        case STATE_CMD1: // 到达预设角度
            switch(sub_state)
            {
                case 0: // 转至设定角度
                    target_angle = M1_angle;
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
                    target_angle = M3_angleA;
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
                        target_angle = M3_angleB;
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

        case STATE_CMD4:
            switch(sub_state)
            {
            case 0: // 前往起点A
                is_speed_mode = 0;  // 角度模式
                target_angle = M4_angleA;   // 预设角度 45~135
                if( fabsf(target_angle - current_angle) < ANGLE_TOLERANCE )
                {
                    sub_state = 1;
                }
                break;

            case 1: // 起步阶段
                target_angle =M4_angleB;  
                omega_cmd = 15.0f;      // 角速度15deg/s

                is_speed_mode = 1;  // 速度模式
                sub_state = 2;
                break;

            case 2: // 匀速阶段
               {
                float breaking_distance = 10.0f;
                if(fabsf(target_angle - current_angle) <= breaking_distance)
                {
                    is_speed_mode = 0;  // 切换角度模式
                    sub_state = 3;
                }
                break;
               }

            case 3: // 停止阶段
                if(fabsf(target_angle - current_angle) < ANGLE_TOLERANCE)
                {
                    Light_Sound_Start(800);
                    sub_state = 4;
                }
                break;

            case 4: // 复位
                if(Light_Sound_Done())
                {
                    current_state = STATE_IDLE;
                }
                break;
            }
            break;
    }
    
}
