#include "stm32f10x.h"
#include "Bluetooth.h"
#include "Delay.h"

// 用 USART3，引脚 PB10(TX), PB11(RX)
#define BT_USART   USART3
#define BT_GPIO    GPIOB
#define BT_TX_PIN  GPIO_Pin_10
#define BT_RX_PIN  GPIO_Pin_11

static uint8_t rxBuffer = 0;
static uint8_t rxReady = 0;

void Bluetooth_Init(void)
{
    // 1. 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    // 2. TX (PB10)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = BT_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BT_GPIO, &GPIO_InitStructure);
    
    // 3. RX (PB11)
    GPIO_InitStructure.GPIO_Pin = BT_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BT_GPIO, &GPIO_InitStructure);
    
    // 4. 串口配置
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(BT_USART, &USART_InitStructure);
    
    // 5. 使能串口
    USART_Cmd(BT_USART, ENABLE);
    
    // 6. 开启接收中断
    USART_ITConfig(BT_USART, USART_IT_RXNE, ENABLE);
    
    // 7. 中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

void Bluetooth_SendString(char *str)
{
    while(*str)
    {
        USART_SendData(BT_USART, *str++);
        while(USART_GetFlagStatus(BT_USART, USART_FLAG_TXE) == RESET);
    }
}

uint8_t Bluetooth_GetChar(void)
{
    if(rxReady)
    {
        rxReady = 0;
        return rxBuffer;
    }
    return 0;
}

uint8_t Bluetooth_IsData(void)
{
    return rxReady;
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(BT_USART, USART_IT_RXNE) == SET)
    {
        rxBuffer = USART_ReceiveData(BT_USART);
        rxReady = 1;
        USART_ClearITPendingBit(BT_USART, USART_IT_RXNE);
    }
}
