#include "stm32f10x.h"
#include "sysTick.h"
#include "UART.h"
#include "KEY.h"
#include "OLED.h"
#include "ADC.h"
#include "DHT11.h"
#include "Encoder.h"
#include "app_data.h"
#include "app_state.h"
#include "app_uart_protocol.h"
#include "bsp_flash.h"
#include "bsp_pwm.h"
#include "buzzer.h"
#include "servo.h"
#include "actuator.h"
#include <stdio.h>

// ===== 土壤湿度校准 =====
#define SOIL_DRY_ADC    4033
#define SOIL_WET_ADC    1539

// ===== 全局变量 =====
DHT11_Data_t g_dht;
uint16_t g_light = 0;
uint16_t g_soil = 0;
uint8_t g_soil_percent = 0;
char buffer[64];

// ===== 时间戳 =====
static uint32_t last_adc = 0;
static uint32_t last_dht = 0;

// ===== 传感器采集任务 =====
void SensorTask(void)
{
    // 每 500ms 读 ADC
    if (GetSysTick() - last_adc >= 500) {
        last_adc = GetSysTick();
        
        g_light = ADC1_ReadChannel_Average(ADC_Channel_0, 10);
        g_soil  = ADC1_ReadChannel_Average(ADC_Channel_1, 10);
        
        // 换算土壤湿度百分比
        if (g_soil <= SOIL_WET_ADC) g_soil_percent = 100;
        else if (g_soil >= SOIL_DRY_ADC) g_soil_percent = 0;
        else g_soil_percent = (SOIL_DRY_ADC - g_soil) * 100 / (SOIL_DRY_ADC - SOIL_WET_ADC);
        
        // 同步到全局结构体
        g_sensor.light = g_light;
        g_sensor.soil  = g_soil_percent;
        
        g_sensor_ready = 1;
    }
    
    // 每 2s 读 DHT11
    if (GetSysTick() - last_dht >= 2000) {
        if (DHT11_Read(&g_dht) == 1) {
            last_dht = GetSysTick();
            g_sensor.temp = g_dht.temp_int;
            g_sensor.humi = g_dht.humi_int;
        } else {
            last_dht = GetSysTick() - 1500;   // 失败后 500ms 重试
        }
    }
    
    // 更新状态等级
    Data_UpdateStatus();
    
    // 状态变化上报
    Data_ReportStatusChange();
    
    // 执行器处理（光照 PWM）
    Data_ActuatorHandler();
    
    // 自动控制（风扇/雾化器/水泵）
    AutoControl();
}

int main(void)
{
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SysTick_Init();
    UART1_Init();
    KEY_Init();
    ADC1_Init();
    DHT11_Init();
    Encoder_Init();
    Flash_LoadThreshold();
    StateMachine_Init();
    PWM_Init();
    Buzzer_Init();
    Servo_Init();
    Actuator_Init();
    
    // OLED 供电
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_SetBits(GPIOB, GPIO_Pin_6);
    DelayMs(100);
    
    OLED_Init();
    OLED_ShowString(1, 1, "Smart Flower");
    
    UART1_SendString("System Ready!\r\n");
    
    while(1) {
		UART_CheckFrame();
        SensorTask();               // 传感器采集 + 自动控制
        StateMachine_Run();         // 状态机（显示 + 按键 + 编码器）
        UART_Protocol_Handler();    // 串口指令解析
    }
}
