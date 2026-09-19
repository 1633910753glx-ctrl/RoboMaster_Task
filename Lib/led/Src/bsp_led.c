#include "bsp_led.h"
#include "tim.h"

void BSP_LED_Init(void)
{
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1); // 蓝灯 PH10
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2); // 绿灯 PH11
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3); // 红灯 PH12
}

void BSP_LED_SetColor(uint16_t red, uint16_t green, uint16_t blue)
{
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, red);   // 红
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, green); // 绿
  __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, blue);  // 蓝
}