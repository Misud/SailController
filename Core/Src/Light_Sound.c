/*
    Light_Sound
    用于管理LED和蜂鸣器
*/
#include "Light_Sound.h"
#include "gpio.h"

static uint16_t led_tick = 0;               // 计数器
static uint8_t  led_state = 0;              // LED状态 0 = 熄灭 1 = 点亮
static uint8_t  led_blink_en = 0;           // 0 = 不开启闪烁 1 = 开启闪烁

// 点亮LED
void LED_ON(void)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_RESET);

}

// 熄灭LED
void LED_OFF(void)
{
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_SET);

}

/*==========LED双闪==========*/
static uint8_t flash_en = 1;                        // 使能双闪
static uint8_t flash_step = 0;                      // 步骤0~4
static uint8_t flash_tick = 0;                      // 计数器

// 触发1次双闪
void LED_DoubleFlash_Start(void)
{
    flash_en    = 1;
    flash_step  = 0;
    flash_tick  = 0;
}

// 进行双闪状态更新，在TIM1 1ms 中断中调用
void LED_DoubleFlash_Update(void)
{
    if(!flash_en) return;       // 未使能双闪，返回

    flash_tick++;               // 每次中断使 双闪计数器 +1

    if(flash_tick >= 200)       // 步长200ms
    {
        flash_tick = 0;         // 重置计数器
        flash_step++;           // 向下1步

        switch(flash_step){
            case 1: LED_ON();   break;  // 第1步：点亮
            case 2: LED_OFF();  break;  // 第2步：熄灭
            case 3: LED_ON();   break;  //        点亮
            case 4: LED_OFF();  break;  //        熄灭
            default:
                flash_en = 0;           // 结束双闪
                flash_step = 0;
                break;
        }
    }

}

// 查询双闪是否结束
uint8_t LED_DoubleFlash_Done(void)
{
    return (flash_en == 0);
}