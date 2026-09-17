#include "ir.h"

#define IR_PIN     GPIO_Pin_5    // 你实际接的引脚
#define IR_PORT    GPIOB
#define IR_CLK     RCC_APB2Periph_GPIOB

void IR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(IR_CLK, ENABLE);
    
    GPIO_InitStruct.GPIO_Pin = IR_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;   // 上拉输入（红外通常低电平触发）
    GPIO_Init(IR_PORT, &GPIO_InitStruct);
}

uint8_t IR_Detected(void)
{
    // ?? 这里根据你的模块实际逻辑改：
    // 低电平触发 → 用 Bit_RESET
    // 高电平触发 → 用 Bit_SET
    return (GPIO_ReadInputDataBit(IR_PORT, IR_PIN) == Bit_SET);
}
