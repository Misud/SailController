/*
    Light_Sound
    用于管理LED和蜂鸣器
*/
#include "Light_Sound.h"
#include "gpio.h"
#include "tim.h"


/*==========LED双闪==========*/
static uint8_t flash_en = 0;                        // 使能双闪
static uint8_t flash_step = 0;                      // 步骤0~4
static uint8_t flash_tick = 0;                      // 计数器

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

// 触发1次双闪
void LED_DoubleFlash_Start(void)
{
    flash_en    = 1;
    flash_step  = 0;
    flash_tick  = 0;
}

// 进行双闪状态更新，在TIM1 1ms 中断中调用
void LED_DoubleFlash_Update(uint16_t buzzer_duration)
{
    if(!flash_en) return;       // 未使能双闪，返回

    flash_tick++;               // 每次中断使 双闪计数器 +1

    if(flash_tick >= (buzzer_duration/4.0))       // 步长 buzzer_duration / 4
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


/*==========蜂鸣器响应==========*/
static uint8_t buzzer_en = 0;                           // 使能蜂鸣器
static uint16_t buzzer_tick = 0;                        // 计时器
static uint16_t buzzer_duration = 0;                    // 蜂鸣时长（ms）

// 开始一次蜂鸣器发声
void Buzzer_Start(uint16_t duration_ms)
{
    buzzer_en = 1;
    buzzer_tick = 0;
    buzzer_duration = duration_ms;
    Buzzer_On();
}

// 打开蜂鸣器 PWM
void Buzzer_On(void)
{
    // 设置蜂鸣器频率为 2k Hz
    __HAL_TIM_SET_PRESCALER(&htim2, 71);
    __HAL_TIM_SET_AUTORELOAD(&htim2, 499);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 250);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

}

// 关闭蜂鸣器 PWM
void Buzzer_Off(void)
{
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

    // 恢复至 20kHz
    __HAL_TIM_SET_PRESCALER(&htim2, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim2, 3599);
}

// 在TIM1 1kHz中断中调用
void Buzzer_Update(void)
{
    if(!buzzer_en)  return;     // 未使能蜂鸣器，返回

    buzzer_tick++;
    if(buzzer_tick >= buzzer_duration)      // 持续 buzzer_duration ms
    {
        buzzer_tick = 0;        // 重置计时器
        buzzer_en = 0;          // 结束蜂鸣器
        Buzzer_Off();           // 关闭 PWM
    }
}

// 查询蜂鸣是否结束
uint8_t Buzzer_Done(void)
{
    return (buzzer_en == 0);
}


/*==========声光提示==========*/
static uint8_t light_sound_en = 0;                              // 使能声光提示

// 进行一次声光提示
void Light_Sound_Start(uint16_t duration_ms)
{
    light_sound_en = 1;

    LED_DoubleFlash_Start();    // 启动LED双闪
    Buzzer_Start(800);             // 启动蜂鸣器
}

// 在 TIM1 1kHz 中断中调用
void Light_Sound_Update(void)
{
    if(!light_sound_en) return;     // 未使能， 返回

    // LED_DoubleFlash_Update 和 Buzzer_Update 自动更新两者状态
    // 这里只需判断两者是否都结束
    if(LED_DoubleFlash_Done() && Buzzer_Done())
    {
        light_sound_en  = 0;
    }
} 

// 查询声光提示是否结束
uint8_t Light_Sound_Done(void)
{
    return (light_sound_en == 0);
}
