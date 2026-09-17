#include "actuator.h"
#include "app_data.h"
#include "app_uart_protocol.h"

// 引脚定义
#define PUMP_PIN         GPIO_Pin_13   // PB13 → 水泵
#define HUMIDIFIER_PIN   GPIO_Pin_14   // PB14 → 雾化器
#define FAN_PIN          GPIO_Pin_0   // PB0 → 风扇
#define ACTUATOR_PORT    GPIOB
#define ACTUATOR_CLK     RCC_APB2Periph_GPIOB

void Actuator_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能 GPIOB + AFIO 时钟（禁用 JTAG）
    RCC_APB2PeriphClockCmd(ACTUATOR_CLK | RCC_APB2Periph_AFIO, ENABLE);
    
    // 关键：禁用 JTAG，保留 SWD，释放 PB13/PB14/PB15
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    // 配置三个引脚为推挽输出
    GPIO_InitStruct.GPIO_Pin = FAN_PIN | HUMIDIFIER_PIN | PUMP_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ACTUATOR_PORT, &GPIO_InitStruct);
    
    // 默认全关（高电平触发 → 低电平是断开）
	GPIO_ResetBits(ACTUATOR_PORT, FAN_PIN | HUMIDIFIER_PIN | PUMP_PIN);
}

// ===== 风扇 =====
void Fan_On(void)  { GPIO_SetBits(ACTUATOR_PORT, FAN_PIN); }
void Fan_Off(void) { GPIO_ResetBits(ACTUATOR_PORT, FAN_PIN); } 

// ===== 雾化器 =====
void Humidifier_On(void)  { GPIO_SetBits(ACTUATOR_PORT, HUMIDIFIER_PIN); }
void Humidifier_Off(void) { GPIO_ResetBits(ACTUATOR_PORT, HUMIDIFIER_PIN); }

// ===== 水泵 =====
void Pump_On(void)  { GPIO_SetBits(ACTUATOR_PORT, PUMP_PIN); }
void Pump_Off(void) { GPIO_ResetBits(ACTUATOR_PORT, PUMP_PIN); }

// ===== 自动控制 =====
void AutoControl(void)
{
	if (UART_GetControlMode() == 1) return;
	
    // 1. 湿度控制
    if (g_sensor.humi > g_threshold.humi_max) {
        // 湿度过高 → 通风，关加湿
        Fan_On();
        Humidifier_Off();
    } else if (g_sensor.humi < g_threshold.humi_min) {
        // 湿度过低 → 加湿，关通风
        Humidifier_On();
        Fan_Off();
    } else {
        // 在范围内 → 都关
        Fan_Off();
        Humidifier_Off();
    }
    
    // 2. 土壤控制
    if (g_sensor.soil > g_threshold.soil_max) {
        // 土壤过湿 → 关泵
        Pump_Off();
    } else if (g_sensor.soil < g_threshold.soil_min) {
        // 土壤过干 → 开泵
        Pump_On();
    } else {
        // 在范围内 → 关泵
        Pump_Off();
    }
}
