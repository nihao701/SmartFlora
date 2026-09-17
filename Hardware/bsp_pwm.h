#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stm32f10x.h"
#include <stdint.h>

void PWM_Init(void);
void PWM_SetDuty(uint8_t duty);   // duty: 0~100
uint8_t PWM_GetDuty(void);

#endif
