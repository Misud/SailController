/*
    I2C
    用于构建软件I2C
*/

#ifndef __I2C_H
#define __I2C_H

#include "main.h"

// 软件 I2C 句柄
typedef struct 
{
    GPIO_TypeDef *SCL_Port;
    uint16_t      SCL_Pin;
    GPIO_TypeDef *SDA_Port;
    uint16_t      SDA_Pin;
}SoftI2C_t;

// 初始化(将 SCL/SDA 拉到空闲高电平)
void SoftI2C_Init(SoftI2C_t *i2c);

// 基础时序
void SoftI2C_Start(SoftI2C_t *i2c);
void SoftI2C_Stop(SoftI2C_t *i2c);
uint8_t SoftI2C_SendByte(SoftI2C_t *i2c, uint8_t byte);     // 0 = ack 1 = nack
uint8_t SoftI2C_ReadByte(SoftI2C_t *i2c, uint8_t ack);      // 1 发 ACK 0 发 NACK

// 寄存器读写
uint8_t SoftI2C_WriteReg(SoftI2C_t *i2c, uint8_t addr, uint8_t reg, uint8_t data);
uint8_t SoftI2C_ReadReg(SoftI2C_t *i2c, uint8_t addr, uint8_t reg);
uint8_t SoftI2C_ReadRegs(SoftI2C_t *i2c, uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

// 扫描总线设备
void SoftI2C_Scan(SoftI2C_t *i2c);

#endif