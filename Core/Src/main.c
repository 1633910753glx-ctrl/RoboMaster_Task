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
#include "can.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// C++混合编译保护罩，防止链接器找不到C函数
#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_led.h"
#include "bsp_servo.h"
#include "justfloat.h"
#include <stdio.h>
  void MX_FREERTOS_Init(void);

#ifdef __cplusplus
}
#endif

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
uint8_t rx_byte;
uint8_t rx_buf[20];
uint8_t rx_cnt = 0;
float kp_val = 50.0f;    // 默认给 50 的起步经验值，防止上电电机毫无反应
Motor_Data_t motor_data; // 电机数据结构体实例，全局变量
float Ki = 0.0f;
float Kd = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*********************************************************
 * [自定义驱动与中断回调区]
 * 用途：存放写的底层外设控制函数和各个通信中断的处理逻辑（回调函数，任务四和任务五）
 *********************************************************/
// 任务一：蜂鸣器音调设置函数
void Buzzer_Set_Tone(uint16_t freq)
{
  if (freq == 0)
  {
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0); // 占空比设为0，静音
    return;
  }
  uint32_t reload = 1000000 / freq; // 1MHz 时钟下的重装载值计算
  __HAL_TIM_SET_AUTORELOAD(&htim4, reload - 1);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, reload / 2); // 始终保持50%占空比
}

// 任务四回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart->Instance == USART1)
  {

    if (rx_byte == '\r')
    {
      rx_buf[rx_cnt] = '\0';

      if (sscanf((char *)rx_buf, "kp=%f", &kp_val) == 1)
      {
        justfloat_send_single(kp_val);
      }
      else if (sscanf((char *)rx_buf, "ki=%f", &Ki) == 1)
      {
        justfloat_send_single(Ki);
      }
      else if (sscanf((char *)rx_buf, "kd=%f", &Kd) == 1)
      {
        justfloat_send_single(Kd);
      }

      rx_cnt = 0;
    }
    else
    {

      rx_buf[rx_cnt++] = rx_byte;
      if (rx_cnt >= 20)
        rx_cnt = 0;
    }

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}
// 任务五回调函数
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  if (hcan->Instance == CAN1)
  {
    // 读取接收到的 CAN 数据
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    // 电机 ID 是 1，由C620 说明书得到其反馈标识符为 0x200 + 1 = 0x201
    if (rx_header.StdId == 0x201)
    {
      // 将 8 个字节的数据拼接还原成真实数值
      motor_data.angle = (rx_data[0] << 8) | rx_data[1];
      motor_data.speed_rpm = (rx_data[2] << 8) | rx_data[3];
      motor_data.current = (rx_data[4] << 8) | rx_data[5];
      motor_data.temp = rx_data[6];
    }
  }
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
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_TIM5_Init();
  MX_TIM1_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
  /*********************************************************
   * [系统初始化与外设启动区]
   * 外设启动(Start)、开启中断(IT)、开机自检。在 FreeRTOS 接管前执行。
   *********************************************************/
  // 局部变量的定义
  CAN_FilterTypeDef can_filter_st;
  // 初始化都写到这里
  BSP_LED_Init();                            // 启动 TIM5 的 PWM 硬件通道
  BSP_LED_SetColor(1000, 0, 0);              // 占空比 1000，这个是测试代码，之前任务二小灯不亮
  Servo_Init();                              // 启动 TIM1 的 PWM 硬件通道
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1); // 开启UART1接收中断

  // 任务一
  // 上电复位后蜂鸣器响一次
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  Buzzer_Set_Tone(4000); // 默认 4000Hz 音调
  HAL_Delay(500);        // 鸣响 500ms
  Buzzer_Set_Tone(0);    // 静音
  HAL_Delay(1000);

  // 设计两种不同的报错音调
  // 报错音调 1：“滴-滴-滴”
  for (int i = 0; i < 3; i++)
  {
    Buzzer_Set_Tone(5000);
    HAL_Delay(150);
    Buzzer_Set_Tone(0);
    HAL_Delay(100);
  }
  HAL_Delay(800);

  // 报错音调 2：“嘟——嘟——”
  for (int i = 0; i < 2; i++)
  {
    Buzzer_Set_Tone(1500);
    HAL_Delay(400);
    Buzzer_Set_Tone(0);
    HAL_Delay(200);
  }
  HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3); // 彻底关闭该通道PWM

  // 任务五
  //  1.配置CAN1的过滤器，允许接收所有ID的报文

  can_filter_st.FilterActivation = CAN_FILTER_ENABLE;
  can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
  can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
  can_filter_st.FilterIdHigh = 0x0000;
  can_filter_st.FilterIdLow = 0x0000;
  can_filter_st.FilterMaskIdHigh = 0x0000;
  can_filter_st.FilterMaskIdLow = 0x0000;
  can_filter_st.FilterBank = 0;

  can_filter_st.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  can_filter_st.SlaveStartFilterBank = 14; //

  HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);

  // 2. 启动 CAN1 硬件模块
  HAL_CAN_Start(&hcan1);

  // 3. 开启 CAN1 的接收中断 (当收到数据时会触发回调函数)
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize(); /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM14 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
