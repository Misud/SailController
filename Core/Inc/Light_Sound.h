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
void LED_DoubleFlash_Update(void);
uint8_t LED_DoubleFlash_Done(void);

#endif