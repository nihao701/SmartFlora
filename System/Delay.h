#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void Delay_us(uint32_t xus);      // 微秒延时
void Delay_ms(uint32_t xms);      // 毫秒延时
void Delay_s(uint32_t xs);        // 秒延时
void SysTime_Init(void);          // 系统时间初始化
uint32_t GetSysTimeMs(void);      // 获取系统运行时间(ms)

#endif
