/*
  MPU6050
  构建卡尔曼滤波器
  卡尔曼滤波主要用于MPU6050角度滤波
*/

#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
#include "I2C.h"

#define MPU6050_ADDR 0x68

// 数据结构体
// 这里只需要 accel_x （+：左偏，-：右偏） gyro_y (+：向右速度，-：向左速度)
typedef struct 
{
  // 加速度部分 单位g
  float accel_x;
  float accel_y;
  float accel_z;

  // 角速度部分 单位 deg/s
  float gyro_x;
  float gyro_y;
  float gyro_z;

  float temperature;  // 温度，单位：℃
}MPU6050_Data_t;

extern float gyro_bias_x;
extern float gyro_bias_y;
extern float gyro_bias_z;

// 初始化
void MPU6050_Init(SoftI2C_t *i2c);

// 获取原始数据
void MPU6050_ReadData(MPU6050_Data_t *data);

// 消除零偏
void MPU6050_Calibrate(uint16_t samples);
 

#endif