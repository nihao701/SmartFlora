#include "servo.h"

// 引脚定义
#define SERVO_PIN     GPIO_Pin_8
#define SERVO_PORT    GPIOA
#define SERVO_CLK     RCC_APB2Periph_GPIOA

void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(SERVO_CLK | RCC_APB2Periph_TIM1, ENABLE);
    
    // 2. PA8 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = SERVO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SERVO_PORT, &GPIO_InitStruct);
    
    // 3. 定时器基础配置：1MHz 计数，周期 20ms
    TIM_TimeBaseStruct.TIM_Prescaler = 72 - 1;      // 72MHz / 72 = 1MHz
    TIM_TimeBaseStruct.TIM_Period = 20000 - 1;      // 20ms
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);
    
    // 4. 输出比较配置（CH1）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 1500;              // 初始 1.5ms → 90°
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStruct);
    
    // 5. 使能预装载
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    
    // 6. ?? 高级定时器必须使能主输出（MOE）
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    // 7. 启动定时器
    TIM_Cmd(TIM1, ENABLE);
}

// 设置角度 0~180°
// 0°   → 500us
// 90°  → 1500us
// 180° → 2500us
void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180;
    uint16_t pulse = 500 + (uint16_t)angle * 2000 / 180;
    TIM_SetCompare1(TIM1, pulse);
}
