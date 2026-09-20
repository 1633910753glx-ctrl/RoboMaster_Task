#include "bsp_servo.h"
#include "tim.h"

void Servo_Init(void)
{
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void Servo_SetAngle(uint8_t angle)
{
  if (angle > 180)
    angle = 180;
  // 0度=500，180度=2500
  uint16_t compare_val = 500 + (uint32_t)angle * 2000 / 180;
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, compare_val);
}