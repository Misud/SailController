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
    static float filtered_derivative = 0;
    filtered_derivative = 0.8f * filtered_derivative + 0.2f * raw_derivative;

    // 总输出
    float pid_feedback = (pid->Kp * pid->err) +
                         (pid->Ki * pid->integral)+
                         (pid->Kd * filtered_derivative);

    pid->last_err = pid->err;

    // 重力前馈
    float pid_ff = 0.0f;
    if(pid->enable_ff)
    {
        float target_rad = target * 3.14159265f / 180.0f;

        // 避开 90° 奇点
        if(target_rad > 1.483f && target_rad < 1.658f)
        {   pid_ff = 0.0f; }
        else
        {   pid_ff = pid->K_ff / tanf(target_rad);  }

        // 前馈限幅
        if(pid_ff > +30.0f)  pid_ff = +30.0f;
        if(pid_ff < -30.0f)  pid_ff = -30.0f;

    }

    // 总输出 = 前馈+反馈
    pid->Output = pid_ff + pid_feedback;

    // 输出限幅
    if(pid->Output > +pid->out_max) pid->Output = +pid->out_max;
    if(pid->Output < -pid->out_max) pid->Output = -pid->out_max;

    return pid->Output;
}