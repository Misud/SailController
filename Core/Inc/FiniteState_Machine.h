#ifndef __FINITE_STATE_MACHINE__
#define __FINITE_STATE_MACHINE__

// 顶层状态机
typedef enum{
    STATE_IDLE = 0,                       // 初始化状态
    STATE_CMD1,                           // 指令1 顶点摆动
    STATE_CMD2,                           // 指令2 
    STATE_CMD3,                           // 指令3
    STATE_CMD4                            // 指令4
}Top_SysState;


// 子状态机 共5个
// STATE_IDLE 下子状态机
typedef enum{
    
}IDLE_STATE;

// STATE_CMD1 下子状态机
typedef enum{
    AngleInput = 0,                    // 输入指定角度
    AngleForward,                      // 摆动到指定角度
    AngleMaintain,                     // 维持指定角度5s
    AngleReset,                        // 返回初始角度
    LightSound                         // 完成后声光提示
}CMD1_STATE;

// STATE_CMD2 下子状态机
typedef enum{
    AngleInit = 0,                     // 角度初始化至45°
    AngleStepping,                     // 以15°步进
    LightSound,                        // 完成步进声提示
    AngleReset                         // 返回初始角度
}CMD2_STATE;

// STATE_CMD3 下子状态机
typedef enum{
    AngleInput = 0,                    // 输入A、B两个指定角度
    NInput,                            // 输入指定次数N
    AngleRoundTrip,                    // 角度往返摆动
    AngleReset                         // 返回初始角度
}CMD3_STATE;

// STATE_CMD4 下子状态机
typedef enum{
    AngleInput = 0,                    // 输入A、B两个指定角度
    AngVelocity,                       // 角速度控制
    AngleReset                         // 返回初始角度                    
}CMD4_STATE;

#endif