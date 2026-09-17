#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include <stdint.h>

// 接收缓冲区（在 UART.c 里定义）
extern uint8_t rx_buffer[128];
extern uint8_t rx_index;
extern uint8_t rx_complete;

void UART1_Init(void);
void UART1_SendByte(uint8_t data);
void UART1_SendString(char *str);
void UART_CheckFrame(void);   

#endif
