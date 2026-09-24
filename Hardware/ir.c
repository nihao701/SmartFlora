#include "ir.h"

#define IR_PIN     GPIO_Pin_9
#define IR_PORT    GPIOB
#define IR_CLK     RCC_APB2Periph_GPIOB

void IR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(IR_CLK, ENABLE);
    
    // HC-SR505 输出高电平，用下拉输入
    GPIO_InitStruct.GPIO_Pin = IR_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;   // 下拉输入
    GPIO_Init(IR_PORT, &GPIO_InitStruct);
}

uint8_t IR_Detected(void)
{
    return (GPIO_ReadInputDataBit(IR_PORT, IR_PIN) == Bit_SET);
}
