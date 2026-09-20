/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_led.h"
#include "bsp_servo.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for ledTask */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for servoTask */
osThreadId_t servoTaskHandle;
const osThreadAttr_t servoTask_attributes = {
  .name = "servoTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartLedTask(void *argument);
void StartServoTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ledTask */
  ledTaskHandle = osThreadNew(StartLedTask, NULL, &ledTask_attributes);

  /* creation of servoTask */
  servoTaskHandle = osThreadNew(StartServoTask, NULL, &servoTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartLedTask */
/**
 * @brief  Function implementing the ledTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartLedTask */
void StartLedTask(void *argument)
{
  /* USER CODE BEGIN StartLedTask */

  uint16_t pwm_val = 0;

  /* Infinite loop */
  for (;;)
  {
    //  红灯渐变
    for (pwm_val = 0; pwm_val <= 1000; pwm_val += 20)
    {
      BSP_LED_SetColor(pwm_val, 0, 0);
      osDelay(10);
    }
    for (pwm_val = 1000; pwm_val > 0; pwm_val -= 20)
    {
      BSP_LED_SetColor(pwm_val, 0, 0);
      osDelay(10);
    }

    for (pwm_val = 0; pwm_val <= 1000; pwm_val += 20)
    {
      BSP_LED_SetColor(0, pwm_val, 0);
      osDelay(10);
    }
    for (pwm_val = 1000; pwm_val > 0; pwm_val -= 20)
    {
      BSP_LED_SetColor(0, pwm_val, 0);
      osDelay(10);
    }

    // 蓝灯渐变
    for (pwm_val = 0; pwm_val <= 1000; pwm_val += 20)
    {
      BSP_LED_SetColor(0, 0, pwm_val);
      osDelay(10);
    }
    for (pwm_val = 1000; pwm_val > 0; pwm_val -= 20)
    {
      BSP_LED_SetColor(0, 0, pwm_val);
      osDelay(10);
    }
    osDelay(1); // 绝对延时 1ms
  }
  /* USER CODE END StartLedTask */
}

/* USER CODE BEGIN Header_StartServoTask */
/**
 * @brief Function implementing the servoTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartServoTask */
void StartServoTask(void *argument)
{
  /* USER CODE BEGIN StartServoTask */

  /* Infinite loop */
  for (;;)
  {
    // 从 0 循环到 180，每次增加 45 度
    for (uint16_t current_angle = 0; current_angle <= 180; current_angle += 45)
    {
      Servo_SetAngle(current_angle);
      osDelay(1000); // 绝对延时 1 秒 (1000ms)
    }
    osDelay(1); // 绝对延时 1ms
  }
  /* USER CODE END StartServoTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

