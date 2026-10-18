/*
    KeyScan
    用于管理按键扫描相关的代码 
*/

#include "KeyScan.h"
#include "gpio.h"

// 按键状态枚举
typedef enum{
    KEY_STATE_IDLE = 0,     // 松开状态
    KEY_STATE_PRESSED       // 按下确认状态
}KeyState_t;

// 按键结构体
typedef struct{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    KeyValue_t key_val;     // 按键对应返回值
    KeyState_t key_state;   // 按键状态
}Key_TypeDef;

// 初始化4个按键
static Key_TypeDef keys[4] = {
    {GPIOB, GPIO_PIN_12, Key_1, KEY_STATE_IDLE},
    {GPIOB, GPIO_PIN_13, Key_2, KEY_STATE_IDLE},
    {GPIOB, GPIO_PIN_14, Key_3, KEY_STATE_IDLE},
    {GPIOB, GPIO_PIN_15, Key_4, KEY_STATE_IDLE}
};

// 按键扫描 (在 TIM1 中断中调用)
KeyValue_t Key_Scan(void)
{
    static uint8_t scan_tick = 0;

    // 降频：每 4 次中断扫描一次（假设 TIM1 是 5ms，即 20ms 扫描一次）
    scan_tick++;
    if(scan_tick < 4) return Key_None;
    scan_tick = 0;

    // 遍历4个按键
    for(int i = 0; i < 4; i++)
    {
        // 0 = 按下, 1 = 松开
        uint8_t is_pressed = (HAL_GPIO_ReadPin(keys[i].GPIOx, keys[i].GPIO_Pin) == GPIO_PIN_RESET);

        if(is_pressed)
        {
            // 如果当前是松开状态，说明刚刚被按下（上升沿/下降沿检测）
            if(keys[i].key_state == KEY_STATE_IDLE)
            {
                keys[i].key_state = KEY_STATE_PRESSED; // 标记为已按下
                return keys[i].key_val;                // 触发一次返回
            }
        }
        else
        {
            // 如果引脚是松开状态，复位标志位
            keys[i].key_state = KEY_STATE_IDLE;
        }
    }

    return Key_None;
}