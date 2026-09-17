#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

#include "stm32f10x.h"

/* 光敏传感器返回值定义
 * 0: 有光 (Bright)
 * 1: 无光 (Dark)
 */

void LightSensor_Init(void);        // 初始化光敏传感器
uint8_t LightSensor_Read(void);      // 读取光敏传感器状态

#endif

