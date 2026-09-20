// 这里为了和c++编译环境匹配
#ifndef __BSP_SERVO_H
#define __BSP_SERVO_H
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void Servo_Init(void);
  void Servo_SetAngle(uint8_t angle);

#ifdef __cplusplus
}
#endif

#endif
