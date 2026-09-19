#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

void BSP_LED_Init(void);
void BSP_LED_SetColor(uint16_t red, uint16_t green, uint16_t blue);

#endif