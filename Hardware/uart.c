#include "stm32f10x.h"
#include "UART.h"
#include "sysTick.h"

void UART1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 115200;   // ? 改成标准波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStruct);
    
    USART_Cmd(USART1, ENABLE);
	// ===== 串口接收中断配置 =====
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;   // 比 SysTick 低
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

// 发送一个字符
void UART1_SendByte(uint8_t data)
{
    while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE)));
    USART_SendData(USART1, data);
}

// 发送字符串（自动处理换行）
void UART1_SendString(char *str)
{
    while(*str)
    {
        if (*str == '\n') {
            UART1_SendByte('\r');   // ? 自动补回车
        }
        UART1_SendByte(*str++);
    }
    // 等待发送完成
    while(!(USART_GetFlagStatus(USART1, USART_FLAG_TC)));
}

// 接收缓冲区
uint8_t rx_buffer[128];
uint8_t rx_index = 0;
uint8_t rx_complete = 0;
static uint32_t rx_last_time = 0;

// 串口中断服务函数
void USART1_IRQHandler(void)
{
    // 先处理溢出错误，防止中断卡死
    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET) {
        USART_ClearFlag(USART1, USART_FLAG_ORE);
        USART_ReceiveData(USART1);
    }
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(USART1);
        
        // 每收到一个字节，更新最后接收时间
        rx_last_time = GetSysTick();
        
        if (rx_index >= sizeof(rx_buffer) - 1) {
            rx_index = 0;   // 防止溢出
        }
        rx_buffer[rx_index++] = ch;
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// ===== 在主循环里调用：检查一帧是否接收完成 =====
void UART_CheckFrame(void)
{
    if (rx_index > 0 && (GetSysTick() - rx_last_time >= 50)) {
        // 50ms 没有新数据，认为一帧结束
        rx_buffer[rx_index] = '\0';
        rx_complete = 1;
        rx_index = 0;
    }
}
