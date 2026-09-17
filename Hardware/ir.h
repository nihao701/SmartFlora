#ifndef __IR_H
#define __IR_H

#include "stm32f10x.h"
#include <stdint.h>

void IR_Init(void);
uint8_t IR_Detected(void);   // 1=检测到人，0=无人

#endif
