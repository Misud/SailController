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
#include "Kalman.h"

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
SoftI2C_t i2c_oled = {GPIOB, GPIO_PIN_6,  GPIOB, GPIO_PIN_7 };
SoftI2C_t i2c_mpu  = {GPIOB, GPIO_PIN_10, GPIOB, GPIO_PIN_11};
volatile MPU6050_Data_t mpu_data;
Kalman_t kalman;
volatile float pitch = 0.0f;
volatile float acc_angle = 0.0f;
volatile float gyro_rate = 0.0f;
volatile KeyValue_t key_from_isr = Key_None;
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

  // 卡尔曼滤波初始化
  Kalman_Init(&kalman);

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
    // 扫描按键
    __disable_irq();
    current_key = key_from_isr;
    key_from_isr = Key_None;
    __enable_irq();

    if(current_key != Key_None)
    {
      switch(current_key)
      {
        case Key_1:key1_cnt++; break;
         case Key_2:key2_cnt++; break;
          case Key_3:key3_cnt++; break;
           case Key_4:key4_cnt--; break;
           default: break;
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    OLED_Clear();

    // 刷新显示
    OLED_ShowString(0, 0, "Long Press_Test");

    if(current_key != Key_None)
    {
      OLED_ShowString(0, 2, "Pressed:");
      OLED_ShowInt(60, 2, current_key);
    }
    else 
    {
      OLED_ShowString(0, 2, "Pressed None!");
    }

    // 显示累计次数
    OLED_ShowString(0, 4, "K1:");
    OLED_ShowInt(30, 4, key1_cnt);

    OLED_ShowString(64, 4, "K2:");
    OLED_ShowInt(94, 4, key2_cnt);

    OLED_ShowString(0, 5, "K3:");
    OLED_ShowInt(32, 5, key3_cnt);

    OLED_ShowString(64, 5, "K4:");
    OLED_ShowInt(94, 5, key4_cnt);

    OLED_Refresh();
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
    // 声光提示更新
    Buzzer_Update();
    LED_DoubleFlash_Update(Light_Sound_GetDuration());
    Light_Sound_Update();

    // MPU6050数据更新
    MPU6050_ReadData((MPU6050_Data_t *)&mpu_data);
    float temp_acc_angle = MPU6050_GetAccAngle((MPU6050_Data_t *)&mpu_data);

    if(temp_acc_angle != 999.0f)
    { acc_angle = temp_acc_angle;   // 数据正常 
    }
    else
    { acc_angle = pitch;            // 数据异常，保持上次数据
    }

    gyro_rate = mpu_data.gyro_y;

    pitch = Kalman_Update(&kalman, acc_angle, gyro_rate, 0.005f);

    // 按键扫描
    KeyValue_t temp_key = Key_Scan();
    if(temp_key != Key_None)
    {
      key_from_isr = temp_key;
    }

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
