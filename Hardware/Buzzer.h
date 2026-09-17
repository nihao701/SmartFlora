#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"

// 蜂鸣器初始化（PA5）
void Buzzer_Init(void);

// 打开蜂鸣器（高电平触发）
void Buzzer_On(void);

// 关闭蜂鸣器
void Buzzer_Off(void);

#endif
