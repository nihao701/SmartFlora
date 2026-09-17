#include "Encoder.h"
#include "sysTick.h"

// 引脚定义
#define ENC_A_PIN       GPIO_Pin_3
#define ENC_B_PIN       GPIO_Pin_4
#define ENC_PORT        GPIOA
#define ENC_CLK         RCC_APB2Periph_GPIOA
#define ENC_EXTI_PORT   GPIO_PortSourceGPIOA
#define ENC_EXTI_PIN    GPIO_PinSource3
#define ENC_EXTI_LINE   EXTI_Line3
#define ENC_IRQn        EXTI3_IRQn

static volatile int16_t enc_count = 0;
static volatile EncoderEvent_t enc_event = ENC_EVENT_NONE;

void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_APB2PeriphClockCmd(ENC_CLK | RCC_APB2Periph_AFIO, ENABLE);
    
    // A 相 PA3 上拉输入
    GPIO_InitStruct.GPIO_Pin = ENC_A_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ENC_PORT, &GPIO_InitStruct);
    
    // B 相 PA4 上拉输入
    GPIO_InitStruct.GPIO_Pin = ENC_B_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ENC_PORT, &GPIO_InitStruct);
    
    // EXTI3 配置（A 相下降沿触发）
    GPIO_EXTILineConfig(ENC_EXTI_PORT, ENC_EXTI_PIN);
    EXTI_InitStruct.EXTI_Line = ENC_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    // NVIC 配置
    NVIC_InitStruct.NVIC_IRQChannel = ENC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI3_IRQHandler(void)
{
    static uint32_t last_time = 0;
    uint32_t now = GetSysTick();
    
    if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
        // 两次中断间隔 < 1ms，认为是抖动，忽略
        if (now - last_time < 1) {
            EXTI_ClearITPendingBit(EXTI_Line3);
            return;
        }
        last_time = now;
        
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == Bit_SET) {
            enc_count++;
        } else {
            enc_count--;
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

EncoderEvent_t Encoder_GetEvent(void)
{
    EncoderEvent_t evt = enc_event;
    enc_event = ENC_EVENT_NONE;
    return evt;
}

int16_t Encoder_GetCount(void)
{
    return enc_count;
}

void Encoder_ResetCount(void)
{
    enc_count = 0;
}
