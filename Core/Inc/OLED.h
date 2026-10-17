/*
    OLED
    用于管理OLED相关代码
*/
#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "I2C.h"

#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// 初始化
void OLED_Init(SoftI2C_t *i2c);

// 清屏
void OLED_Clear(void);

// 全亮
void OLED_TestAllOn(void);

// 显示字符/字符串
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

// 显示数字（带小数）
void OLED_ShowInt(uint8_t x, uint8_t y, int32_t num);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t decimals);

// 刷新（把缓冲区写到屏幕）
void OLED_Refresh(void);

// 更高速的刷新
static void OLED_WriteDataBulk(uint8_t *buf, uint16_t len);


#endif