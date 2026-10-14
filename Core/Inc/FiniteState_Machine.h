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
// STATE_IDLE 子状态机
typedef enum{
    
}IDLE_STATE;

// STATE_CMD1 子状态机
typedef enum{

}CMD1_STATE;

// STATE_CMD2 子状态机
typedef enum{

}CMD2_STATE;

// STATE_CMD3 子状态机
typedef enum{

}CMD3_STATE;

// STATE_CMD4 子状态机
typedef enum{

}CMD4_STATE;

#endif