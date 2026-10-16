/*
    I2C
    用于搭建软件I2C架构
*/
#include "I2C.h"
#include <stdio.h>

/*==========内部工具==========*/

// 延时，约100kHz
static void i2c_delay(void)
{
    for(volatile int i = 0; i < 50; i++);
}

// 引脚电平操作
static inline void SCL_H(SoftI2C_t* i2c)
{
    HAL_GPIO_WritePin(i2c->SCL_Port, i2c->SCL_Pin, GPIO_PIN_SET);
}
static inline void SCL_L(SoftI2C_t* i2c)
{
    HAL_GPIO_WritePin(i2c->SCL_Port, i2c->SCL_Pin, GPIO_PIN_RESET);
}
static inline void SDA_H(SoftI2C_t* i2c)
{
    HAL_GPIO_WritePin(i2c->SDA_Port, i2c->SDA_Pin, GPIO_PIN_SET);
}
static inline void SDA_L(SoftI2C_t* i2c)
{
    HAL_GPIO_WritePin(i2c->SDA_Port, i2c->SDA_Pin, GPIO_PIN_RESET);
}
static inline uint8_t SDA_READ(SoftI2C_t *i2c)
{
    return HAL_GPIO_ReadPin(i2c->SDA_Port, i2c->SDA_Pin);
}


/*==========初始化==========*/
void SoftI2C_Init(SoftI2C_t* i2c)
{
    SCL_H(i2c);
    SDA_H(i2c);
}


/*==========起始条件==========*/
void SoftI2C_Start(SoftI2C_t* i2c)
{
    SDA_H(i2c);             // 保证 SDA 为高
    SCL_H(i2c);             // SCL拉高
    i2c_delay();
    SDA_L(i2c);             // SCL为高时， SDA由高变低
    i2c_delay();
    SCL_L(i2c);             // 拉低SCL准备传输数据
    i2c_delay();
}


/*==========停止条件==========*/
void SoftI2C_Stop(SoftI2C_t* i2c)
{
    SDA_L(i2c);             // 保证 SDA 为低
    SCL_H(i2c);             // SCL拉高
    i2c_delay();
    SDA_H(i2c);             // SCL高时，SDA由低变高
    i2c_delay();
}


/*==========发送一个字节==========*/
// 返回 0 = 收到 ACK，1 = NACK
uint8_t SoftI2C_SendByte(SoftI2C_t* i2c, uint8_t byte)
{
    for(int i = 0; i < 8; i++)
    {
        if(byte & 0x80) SDA_H(i2c); // 取最高位
        else            SDA_L(i2c);
        byte <<= 1;                 // 左移，准备下一位
        i2c_delay();
        SCL_H(i2c);                 // SCL高，从机采样
        i2c_delay();
        SCL_L(i2c);                 // SCL低，允许SDA变化
        i2c_delay();
    }

    // 第九个时钟 读应答ack
    SDA_H(i2c);                      // 主机释放SDA
    i2c_delay();
    SCL_H(i2c);                      // 从机拉低或释放
    i2c_delay();
    uint8_t ack = SDA_READ(i2c);     // 读 SDA：0 = ACK; 1 = NACK        
    SCL_L(i2c);
    i2c_delay();
    return ack;
}


/*==========接收一个字节==========*/
// ack = 1 发送ACK; ack = 0 发送NACK
uint8_t SoftI2C_ReadByte(SoftI2C_t* i2c, uint8_t ack)
{
    uint8_t byte = 0;
    SDA_H(i2c);                 // 释放SDA
    for(int i = 0; i < 8; i++)
    {
        byte <<= 1;
        i2c_delay();
        SCL_H(i2c);                     // SCL高，从机输出数据
        i2c_delay();
        if(SDA_READ(i2c)) byte |= 1;    // 主机采样
        SCL_L(i2c);                     // SCL低，从机换下一位
        i2c_delay();
    }

    // 发送 ACK/NACK 
    if(ack) SDA_L(i2c);                 // ACK:拉低
    else    SDA_H(i2c);                 // NACK:释放
    i2c_delay();
    SCL_H(i2c);
    i2c_delay();
    SCL_L(i2c);
    i2c_delay();
    SDA_H(i2c);                         // 释放SDA
    return byte;
}


/*==========写寄存器==========*/
uint8_t SoftI2C_WriteReg(SoftI2C_t* i2c, uint8_t addr, uint8_t reg, uint8_t data)
{
    SoftI2C_Start(i2c);
    if (SoftI2C_SendByte(i2c, (addr << 1) | 0)  != 0)
    {
        SoftI2C_Stop(i2c);
        return 1;
    }
    if(SoftI2C_SendByte(i2c, reg)               != 0)
    {
        SoftI2C_Stop(i2c);
        return 1;
    }
    if(SoftI2C_SendByte(i2c, data)              != 0)
    {
        SoftI2C_Stop(i2c);
        return 1;
    }
    SoftI2C_Stop(i2c);
    return 0;
}


/*==========读寄存器==========*/
uint8_t SoftI2C_ReadReg(SoftI2C_t* i2c, uint8_t addr, uint8_t reg)
{
    uint8_t data;
    SoftI2C_Start(i2c);
    SoftI2C_SendByte(i2c, (addr << 1) | 0);
    SoftI2C_SendByte(i2c, reg);
    SoftI2C_Start(i2c);                         // 重复起始
    SoftI2C_SendByte(i2c, (addr << 1) | 1);
    data = SoftI2C_ReadByte(i2c, 0);            // 发NACK
    SoftI2C_Stop(i2c);
    return data;
}


/*==========连续读多个寄存器==========*/
uint8_t SoftI2C_ReadRegs(SoftI2C_t* i2c, uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
{
    SoftI2C_Start(i2c);
    SoftI2C_SendByte(i2c, (addr << 1) | 0);
    SoftI2C_SendByte(i2c, reg);
    SoftI2C_Start(i2c);
    SoftI2C_SendByte(i2c, (addr << 1) | 1);
    for(uint8_t i = 0; i < len; i++)
    {
        buf[i] = SoftI2C_ReadByte(i2c, (i < len - 1) ? 1 : 0);
    }
    SoftI2C_Stop(i2c);
    return 0;
}

/*==========扫描总线==========*/
void SoftI2C_Scan(SoftI2C_t *i2c)
{
    printf("I2C Scan:\r\n");
    for(uint8_t addr = 1; addr < 128; addr++)
    {
        SoftI2C_Start(i2c);
        uint8_t ack = SoftI2C_SendByte(i2c, (addr << 1) | 0);
        SoftI2C_Stop(i2c);
        if(ack == 0)
        {
            printf("  Found: 0x%02X\r\n", addr);
        }
    }
    printf("Scan done.\r\n");
}