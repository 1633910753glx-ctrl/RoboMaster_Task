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
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_led.h"
#include "bsp_servo.h"
#include "can.h"
#include "justfloat.h"

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
/* USER CODE BEGIN Variables */
// 全局变量声明
extern Motor_Data_t motor_data;
extern float kp_val; // 比例系数(P)，初始默认50.0，在main.c串口中断(任务四)中动态修改
extern float Ki;
extern float Kd;
/* USER CODE END Variables */
/* Definitions for ledTask */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
    .name = "ledTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for servoTask */
osThreadId_t servoTaskHandle;
const osThreadAttr_t servoTask_attributes = {
    .name = "servoTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for MotorTask */
osThreadId_t MotorTaskHandle;
const osThreadAttr_t MotorTask_attributes = {
    .name = "MotorTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartLedTask(void *argument);
void StartServoTask(void *argument);
void StartMotorTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
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

  /* creation of MotorTask */
  MotorTaskHandle = osThreadNew(StartMotorTask, NULL, &MotorTask_attributes);

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

  uint16_t pwm_val = 0; // 用于存储PWM占空比的变量，范围从0到1000，表示LED的亮度级别。

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

/* USER CODE BEGIN Header_StartMotorTask */
/**
 * @brief Function implementing the MotorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN StartMotorTask */
  float error = 0.0f;
  float last_error = 0.0f;
  float integral = 0.0f;
  float output_current = 0.0f;

  static uint32_t traj_tick = 0;
  float target_angle = 0.0f;

  int16_t diff = 0;
  static int16_t last_angle = 0;
  static int32_t total_angle = 0;
  float out_angle_deg = 0.0f;

  /* Infinite loop */
  for (;;)
  {
    // 轨迹规划 (周期: 2000ms)
    uint32_t cycle_tick = traj_tick % 200;

    if (cycle_tick < 50)
    {
      target_angle = 90.0f * ((float)cycle_tick / 50.0f);
    }
    else if (cycle_tick < 150)
    {
      uint32_t t = cycle_tick - 50;
      target_angle = 90.0f - 180.0f * ((float)t / 100.0f);
    }
    else
    {
      uint32_t t = cycle_tick - 150;
      target_angle = -90.0f + 90.0f * ((float)t / 50.0f);
    }
    traj_tick++;

    // 编码器数据处理与越界校验
    diff = motor_data.angle - last_angle;
    if (diff < -4096)
    {
      diff += 8192;
    }
    else if (diff > 4096)
    {
      diff -= 8192;
    }

    total_angle += diff;
    last_angle = motor_data.angle;
    out_angle_deg = (float)total_angle * 360.0f / (8192.0f * 19.2f);

    // PID 闭环计算
    error = target_angle - out_angle_deg;
    integral += error;

    // 积分项限幅
    if (integral > 10000.0f)
    {
      integral = 10000.0f;
    }
    else if (integral < -10000.0f)
    {
      integral = -10000.0f;
    }

    output_current = (kp_val * error) + (Ki * integral) + (Kd * (error - last_error));
    last_error = error;

    // 总输出电流限幅
    if (output_current > 8000.0f)
    {
      output_current = 8000.0f;
    }
    else if (output_current < -8000.0f)
    {
      output_current = -8000.0f;
    }

    // 发送电机控制指令与波形数据
    CAN_cmd_motor((int16_t)output_current, 0, 0, 0);
    justfloat_send_3(output_current, out_angle_deg, motor_data.speed_rpm);

    osDelay(10);
  }
  /* USER CODE END StartMotorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
