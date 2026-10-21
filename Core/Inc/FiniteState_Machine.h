#ifndef __FINITE_STATE_MACHINE_H__
#define __FINITE_STATE_MACHINE_H__

// 顶层状态机
typedef enum{
    STATE_IDLE = 0,                       // 初始化状态
    STATE_CMD1,                           // 指令1 顶点摆动
    STATE_CMD2,                           // 指令2 
    STATE_CMD3,                           // 指令3
    STATE_CMD4                            // 指令4
}SystemState_t;


// 状态机初始化
void FSM_Init(void);

// 状态机更新
void FSM_Update(void);

#endif