#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

// ===== 按键1（KEY1）引脚：PB1 =====
#define KEY1_PIN    GPIO_Pin_1
#define KEY1_PORT   GPIOB
#define KEY1_CLK    RCC_APB2Periph_GPIOB

// ===== 按键2（KEY2）引脚：PB12 =====
#define KEY2_PIN    GPIO_Pin_12
#define KEY2_PORT   GPIOB
#define KEY2_CLK    RCC_APB2Periph_GPIOB

// 按键事件枚举
typedef enum {
    KEY_EVENT_NONE,      // 无事件
    KEY_EVENT_SHORT,     // 短按（<1秒）
    KEY_EVENT_LONG,      // 长按（>=1秒）
} KeyEvent_t;

// 初始化两个按键（PB1、PB12）
void KEY_Init(void);

// 获取 KEY1 事件（读取后自动清除）
KeyEvent_t KEY1_GetEvent(void);

// 获取 KEY2 事件（读取后自动清除）
KeyEvent_t KEY2_GetEvent(void);

#endif
