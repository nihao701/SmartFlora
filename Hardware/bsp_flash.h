#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include "stm32f10x.h"
#include <stdint.h>

#define FLASH_SAVE_ADDR   0x0800FC00   // 最后一页起始地址

void Flash_SaveThreshold(void);
void Flash_LoadThreshold(void);

#endif
