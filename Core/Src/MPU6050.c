/*
  MPU6050
  用于管理MPU6050的相关代码
*/

#include "MPU6050.h"
#include <math.h>

static SoftI2C_t *mpu_i2c;

// MPU6050寄存器地址
#define SMPLRT_DIV      0x19
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H    0x3B
#define PWR_MGMT_1      0x6B
#define WHO_AM_I        0x75


/*==========初始化MPU6050==========*/
void MPU6050_Init(SoftI2C_t *i2c)
{
  mpu_i2c = i2c;

  // 接触休眠 设置时钟源为 X 轴陀螺仪 PLL
  SoftI2C_WriteReg(mpu_i2c, MPU6050_ADDR, PWR_MGMT_1, 0x01);
  HAL_Delay(50);    // 等待芯片内部起振

  // 设置采样率分频（1kHz / (1 + 9) = 100Hz）
  SoftI2C_WriteReg(mpu_i2c, MPU6050_ADDR, SMPLRT_DIV, 0x09);

  // 配置数字低通滤波器，带宽约44kHz
  SoftI2C_WriteReg(mpu_i2c, MPU6050_ADDR, CONFIG, 0x03);

  // 设置陀螺仪量程：±250°/s
  SoftI2C_WriteReg(mpu_i2c, MPU6050_ADDR, GYRO_CONFIG, 0x00);

  // 设置加速度计量程：±2g
  SoftI2C_WriteReg(mpu_i2c, MPU6050_ADDR, ACCEL_CONFIG, 0X00);
}



// 零偏值（单位：deg/s）
float gyro_bias_x = 0, gyro_bias_y = 0, gyro_bias_z = 0;

/*==========读取MPU6050数据并进行物理量转换==========*/
void MPU6050_ReadData(MPU6050_Data_t *data)
{
  uint8_t buf[14];                  // 连续读取14个寄存器的数据的缓冲区
  int16_t raw_ax, raw_ay, raw_az;   
  int16_t raw_gx, raw_gy, raw_gz;
  int16_t raw_temp;

  // 从 0x3B 开始连续读取14个字节
  SoftI2C_ReadRegs(mpu_i2c, MPU6050_ADDR, ACCEL_XOUT_H, buf, 14);

  // 拼接高8位和低8位，获取原始数据
  raw_ax = (int16_t)((buf[0] << 8) | buf[1]); 
  raw_ay = (int16_t)((buf[2] << 8) | buf[3]); 
  raw_az = (int16_t)((buf[4] << 8) | buf[5]); 

  raw_temp = (int16_t)((buf[6] << 8) | buf[7]); 

  raw_gx = (int16_t)((buf[8] << 8) | buf[9]); 
  raw_gy = (int16_t)((buf[10] << 8) | buf[11]); 
  raw_gz = (int16_t)((buf[12] << 8) | buf[13]); 

  // 数据转换为物理量
  // 加速度 量程：16384 LSB/g
  data->accel_x = raw_ax / 16384.0f;
  data->accel_y = raw_ay / 16384.0f;
  data->accel_z = raw_az / 16384.0f;
  
  // 角速度 量程：131 LSB/(deg/s)
  data->gyro_x = raw_gx / 131.0f - gyro_bias_x;
  data->gyro_y = raw_gy / 131.0f - gyro_bias_y;
  data->gyro_z = raw_gz / 131.0f - gyro_bias_z;

  // 温度 Temp = raw_temp / 340.0 + 36.53
  data->temperature = raw_temp / 340.0f + 36.53f;
}


/*==========消除陀螺仪零偏==========*/
void MPU6050_Calibrate(uint16_t samples)
{
  float sum_gx = 0, sum_gy = 0, sum_gz = 0;
  uint8_t buf[6];
  int16_t raw_gx, raw_gy, raw_gz;

  for(uint16_t i = 0; i < samples; i++)
  {
    SoftI2C_ReadRegs(mpu_i2c, MPU6050_ADDR, 0x43, buf, 6);

    raw_gx = (int16_t)((buf[0] << 8) | buf[1]);
    raw_gy = (int16_t)((buf[2] << 8) | buf[3]);
    raw_gz = (int16_t)((buf[4] << 8) | buf[5]);

    sum_gx += raw_gx / 131.0f;
    sum_gy += raw_gy / 131.0f;
    sum_gz += raw_gz / 131.0f;
    HAL_Delay(5);
  }

  gyro_bias_x = sum_gx / samples;
  gyro_bias_y = sum_gy / samples;
  gyro_bias_z = sum_gz / samples;
}


/*==========获取加速度计角度（含模长限制）==========*/
float MPU6050_GetAccAngle(MPU6050_Data_t *data)
{
  // 计算模长平方
  float acc_norm = data->accel_x * data->accel_x
                 + data->accel_y * data->accel_y
                 + data->accel_z * data->accel_z;

  // 利用acc_norm判断MPU6050是否平滑运动
  if (acc_norm > ACC_NORM_MIN && acc_norm < ACC_NORM_MAX)
  {
    // acc_norm正常 则正常返回值
    return atan2(data->accel_x, data->accel_z) * 57.2958f;
  } 
  else
  {
    // 剧烈运动 返回999.0f 报错
    return 999.0f;
  }
}