#include "justfloat.h"
#include "usart.h"
#include <string.h>

void justfloat_send_single(float value)
{

  float frame[2];

  const uint32_t tail = 0x7F800000u;

  frame[0] = value;

  memcpy(&frame[1], &tail, sizeof(tail));

  HAL_UART_Transmit(&huart1, (uint8_t *)frame, sizeof(frame), 100);
}