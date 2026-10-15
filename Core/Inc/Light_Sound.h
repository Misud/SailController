/*
    Light_Sound
    用于管理LED和蜂鸣器
*/
#ifndef __LIGHT_SOUND_H
#define __LIGHT_SOUND_H

#include "main.h"

// 灯光部分
void LED_ON(void);
void LED_OFF(void);
void LED_DoubleFlash_Start(void);
void LED_DoubleFlash_Update(uint16_t buzzer_duration);
uint8_t LED_DoubleFlash_Done(void);

// 蜂鸣器部分
void Buzzer_Start(uint16_t duration_ms);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Update(void);
uint8_t Buzzer_Done(void);

//声光提示
void Light_Sound_Start(uint16_t duration_ms);
void Light_Sound_Update(void);
uint8_t Light_Sound_Done(void);

#endif