#include "bsp_pwm.h"

// 引脚定义
#define PWM_PIN      GPIO_Pin_7
#define PWM_PORT     GPIOA
#define PWM_CLK      RCC_APB2Periph_GPIOA

// PWM 参数
#define PWM_ARR      1000    // 自动重装载值
#define PWM_PSC      71      // 预分频：72MHz / (71+1) = 1MHz
// PWM 频率 = 1MHz / 1000 = 1kHz

static uint8_t pwm_duty = 0;   // 当前占空比 0~100

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    // 1. 使能时钟（GPIOA + TIM3 + AFIO）
    RCC_APB2PeriphClockCmd(PWM_CLK | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 2. PA7 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = PWM_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PWM_PORT, &GPIO_InitStruct);
    
    // 3. 定时器基础配置
    TIM_TimeBaseStruct.TIM_Prescaler = PWM_PSC;
    TIM_TimeBaseStruct.TIM_Period = PWM_ARR;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
    
    // 4. 输出比较配置（CH2，对应 PA7）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;                        // 初始占空比 0
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM3, &TIM_OCInitStruct);
    
    // 5. 使能预装载
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    
    // 6. 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

// 设置占空比 0~100
void PWM_SetDuty(uint8_t duty)
{
    if (duty > 100) duty = 100;
    pwm_duty = duty;
    TIM_SetCompare2(TIM3, duty * PWM_ARR / 100);
}

uint8_t PWM_GetDuty(void)
{
    return pwm_duty;
}
