#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f10x.h"

void Bluetooth_Init(void);
void Bluetooth_SendString(char *str);
uint8_t Bluetooth_GetChar(void);
uint8_t Bluetooth_IsData(void);

#endif
