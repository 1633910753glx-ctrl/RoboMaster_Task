#ifndef __JUSTFLOAT_H__
#define __JUSTFLOAT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  void justfloat_send_single(float value);             // 发送单个浮点数
  void justfloat_send_3(float v1, float v2, float v3); // 发送三个浮点数

#ifdef __cplusplus
}
#endif

#endif