#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include <stdint.h>

void ADC1_Init(void);
uint16_t ADC1_Read(void);
uint16_t ADC1_Read_Average(uint8_t times);

uint16_t ADC1_ReadChannel(uint8_t channel);
uint16_t ADC1_ReadChannel_Average(uint8_t channel, uint8_t times);

#endif
