/*
    PID
    用于管理PID相关代码
*/

#include "PID.h"
#include <math.h>


// PID初始化
void PID_Init(PID_t *pid, float kp, float ki, float kd, float out_max)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->target         = 0;
    pid->actual         = 0;
    pid->err            = 0;
    pid->last_err       = 0;
    pid->integral       = 0;
    pid->Output         = 0;
    pid->filtered_derivative = 0;

    pid->out_max = out_max;
    pid->integral_max = out_max * 0.5f;
}


// PID计算
float PID_Compute(PID_t *pid, float target, float actual, float dt)
{
    pid->target = target;
    pid->actual = actual;
    pid->err = pid->target - pid->actual;       // 误差值计算

    // 积分计算
    pid->integral += pid->err * dt;
    if(pid->integral > +pid->integral_max)  pid->integral = +pid->integral_max;
    if(pid->integral < -pid->integral_max)  pid->integral = -pid->integral_max;

    // 微分计算
    float raw_derivative = (pid->err - pid->last_err) / dt;

    // 微分低通滤波
    pid->filtered_derivative = 0.8f * pid->filtered_derivative + 0.2f * raw_derivative;

    // 总输出
    float pid_feedback = (pid->Kp * pid->err) +
                         (pid->Ki * pid->integral)+
                         (pid->Kd * pid->filtered_derivative);

    pid->last_err = pid->err;

    // 总输出 = 反馈
    pid->Output = pid_feedback;

    // 输出限幅
    if(pid->Output > +pid->out_max) pid->Output = +pid->out_max;
    if(pid->Output < -pid->out_max) pid->Output = -pid->out_max;

    return pid->Output;
}