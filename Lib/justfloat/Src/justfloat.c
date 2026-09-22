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
// 发送3个浮点数（角度、电流、转速），共用一个尾帧
void justfloat_send_3(float v1, float v2, float v3)
{
  float frame[4]; // 3个数据 + 1个尾帧
  const uint32_t tail = 0x7F800000u;

  frame[0] = v1;
  frame[1] = v2;
  frame[2] = v3;

  // 把尾帧安全地拼接到数组最后
  memcpy(&frame[3], &tail, sizeof(tail));

  // 一次性发送 16 个字节 (4个float * 4)
  HAL_UART_Transmit(&huart1, (uint8_t *)frame, sizeof(frame), 100);
}