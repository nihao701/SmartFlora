#include "Buzzer.h"

// ===== 蜂鸣器引脚定义：PA5 =====
#define BUZZER_PIN    GPIO_Pin_12
#define BUZZER_PORT   GPIOA
#define BUZZER_CLK    RCC_APB2Periph_GPIOA

/**
 * @brief  蜂鸣器初始化
 * @note   PA5 推挽输出，初始关闭
 *         接的是有源蜂鸣器（高电平响）
 *         如果接的是低电平触发的蜂鸣器/LED，把 On/Off 里的 Set/Reset 对调
 */
void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能 GPIOA 时钟
    RCC_APB2PeriphClockCmd(BUZZER_CLK, ENABLE);
    
    // PA5 推挽输出
    GPIO_InitStruct.GPIO_Pin = BUZZER_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
    
    Buzzer_Off();
}

/**
 * @brief  打开蜂鸣器
 */
void Buzzer_On(void)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);   // PA5 = 高电平
}

/**
 * @brief  关闭蜂鸣器
 */
void Buzzer_Off(void)
{
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); // PA5 = 低电平
}
