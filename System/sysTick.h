#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"
#include <stdint.h>

void SysTick_Init(void);
uint32_t GetSysTick(void);
void DelayMs(uint32_t ms);
void DelayUs(uint32_t us);
void SysTick_Handler_Callback(void);   // 新增：给中断调用的回调

#endif
