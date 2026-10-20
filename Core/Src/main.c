/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "Light_Sound.h"
#include "I2C.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Kalman.h"
#include "PID.h"

#include "KeyScan.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// I2C引脚定义
SoftI2C_t i2c_oled = {GPIOB, GPIO_PIN_6,  GPIOB, GPIO_PIN_7 };
SoftI2C_t i2c_mpu  = {GPIOB, GPIO_PIN_10, GPIOB, GPIO_PIN_11};

// MPU6050数据参数
volatile MPU6050_Data_t mpu_data;

// 卡尔曼滤波参数
Kalman_t kalman;
volatile float pitch = 0.0f;
volatile float acc_angle = 0.0f;  
volatile float gyro_rate = 0.0f;

// PID参数
PID_t pid_angle;    // 外环PID
PID_t pid_gyro;     // 内环PID

// 按键检测参数
volatile KeyValue_t key_from_isr = Key_None;

// 电机参数
const float U0 = 50.0f;   // 基础占空比

// 全局参数 
float target_angle = 135.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
  return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  LED_OFF();                      // Ϩ��LED

  // I2C初始化
  SoftI2C_Init(&i2c_oled);
  SoftI2C_Init(&i2c_mpu);

  // OLED初始化
  OLED_Init(&i2c_oled);           // ��ʼ��OLED
  OLED_TestAllOn();
  HAL_Delay(1000);
  OLED_Clear();

  // MPU6050 初始化
  MPU6050_Init(&i2c_mpu);
  MPU6050_Calibrate(200);

  // 电机初始化
  Motor_Init();

  // 卡尔曼滤波初始化
  Kalman_Init(&kalman);

  // PID初始化
  PID_Init(&pid_angle, 2.0f, 0.25f, 1.0f, 20.0f);   // 外环PID，期望角速度±20deg/s // 0.4725f, 0.035f, 0.2025f, 20.0f
  PID_Init(&pid_gyro,  1.5f, 0.0f, 0.5f, 60.0f);   // 内环PID，限制delta_u ±60% // 0.3625f, 0.0f, 0.0045f, 60.0f

  // TIM1 1kHz 初始化
  __HAL_TIM_SET_COUNTER(&htim1, 0); // tim1 计数器从0开始
  HAL_TIM_Base_Start_IT(&htim1);    // tim1 中断初始化

  // 初始化界面展示
  uint8_t who_am_i = SoftI2C_ReadReg(&i2c_mpu, 0x68, 0x75);

  OLED_ShowString(0, 0, "OLED OK!");
  OLED_ShowString(0, 2, "I2C OK!");
  OLED_ShowString(0, 4, "2026-10-16");
  OLED_ShowInt(0, 6, (int32_t)who_am_i);
  OLED_ShowString(25, 6, "MPU6050 OK!");
  OLED_Refresh();                 // ˢ�µ���Ļ
  HAL_Delay(1000);
  OLED_Clear();
  OLED_Refresh();

  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t key1_cnt = 0, key2_cnt = 0, key3_cnt = 0, key4_cnt = 0; 
  KeyValue_t current_key;

  while (1)
  {
    // 屏幕刷新
    OLED_Clear();
    OLED_ShowString(0, 0, "Target:");
    OLED_ShowInt(60, 0, (int)target_angle);

    OLED_ShowString(0, 2, "Current:");
    OLED_ShowFloat(60, 2, 90.0f-pitch, 2);

    OLED_ShowString(0, 4, "PID Out:");
    OLED_ShowFloat(60, 4, pid_gyro.Output, 2);


    OLED_Refresh();
    HAL_Delay(50);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM1)
  {
    /*声光提示更新*/
    Buzzer_Update();
    LED_DoubleFlash_Update(Light_Sound_GetDuration());
    Light_Sound_Update();

    /*MPU6050数据更新，读取姿态*/
    MPU6050_ReadData((MPU6050_Data_t *)&mpu_data);
    float temp_acc_angle = MPU6050_GetAccAngle((MPU6050_Data_t *)&mpu_data);

    if(temp_acc_angle != 999.0f)
    { acc_angle = temp_acc_angle;   // 数据正常 
    }
    else
    { acc_angle = pitch;            // 数据异常，保持上次数据
    }

    float raw_gyro = mpu_data.gyro_y;
    // 角速度低通滤波
    static float filtered_gyro = 0.0f;
    filtered_gyro = 0.7f * filtered_gyro + 0.3f * raw_gyro;
    gyro_rate = filtered_gyro;

    pitch = Kalman_Update(&kalman, acc_angle, gyro_rate, 0.005f);
    //  坐标系修正
    float physical_angle = 90.0f - pitch;

    /*按键扫描*/
    KeyValue_t temp_key = Key_Scan();
    if(temp_key != Key_None)
    {
      key_from_isr = temp_key;
    }

    /*串级PID*/
    float target_omega     = PID_Compute(&pid_angle, target_angle, physical_angle, 0.005f);
    float delta_u_feedback = PID_Compute(&pid_gyro,  target_omega, gyro_rate, 0.005f);

		// 前馈处理
    float target_rad = target_angle * 3.14159f / 180.0f;
    float delta_u_ff = 0.0f;
    if(fabsf(target_rad - 1.5708f) > 0.01f)
    {
      delta_u_ff = -33.0625f / tanf(target_rad) ;
      if(delta_u_ff > +50.0f) delta_u_ff = +50.0f;
      if(delta_u_ff < -50.0f) delta_u_ff = -50.0f;
    }

    float total_delta_u = delta_u_feedback + delta_u_ff;
    //死区屏蔽
    if (total_delta_u > 0.0f && total_delta_u < 2.34375f)  total_delta_u = 0.0f;
    if (total_delta_u < 0.0f && total_delta_u > -2.34375f) total_delta_u = 0.0f;
		
		// 消除机械静差
		total_delta_u += 2.28125f;

    /*分配PWM*/
    float u1 = U0 + (total_delta_u / 2.0f);
    float u2 = U0 - (total_delta_u / 2.0f);

    if(u1 > 100.0f) u1 = 100.0f;
    if(u2 > 100.0f) u2 = 100.0f;
    if(u1 < 0.0f)   u1 = 0.0f;
    if(u2 < 0.0f)   u2 = 0.0f;


    Motor_SetPWM(u1, u2);

  }
  else if(htim->Instance == TIM2)
  {

  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
