#include "ADC.h"

void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;
    
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    
    // 2. 配置 PA0 为模拟输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 3. 复位 ADC1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
    
    // 4. 配置 ADC
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;          // 独立模式
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;               // 不扫描（单通道）
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;         // 不连续（单次转换）
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;      // 数据右对齐
    ADC_InitStruct.ADC_NbrOfChannel = 1;                     // 转换通道数 1
    ADC_Init(ADC1, &ADC_InitStruct);
    
    // 5. 校准
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// 读取单次 ADC 值（0~4095）
uint16_t ADC1_Read(void)
{
    uint16_t value = 0;
    
    // 选择通道 0，采样时间 239.5 周期（最稳）
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
    
    // 启动软件转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // 等待转换完成
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    // 读取结果
    value = ADC_GetConversionValue(ADC1);
    
    return value;
}

// 多次采样取平均（软件滤波，抑制光线抖动）
uint16_t ADC1_Read_Average(uint8_t times)
{
    uint32_t sum = 0;
    uint8_t i;
    
    for (i = 0; i < times; i++) {
        sum += ADC1_Read();
    }
    
    return (uint16_t)(sum / times);
}

// 读取指定通道的 ADC 值（0~4095）
uint16_t ADC1_ReadChannel(uint8_t channel)
{
    // 选择通道，采样时间 239.5 周期
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    // 启动软件转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // 等待转换完成
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    // 读取结果
    return ADC_GetConversionValue(ADC1);
}

// 对指定通道多次采样取平均
uint16_t ADC1_ReadChannel_Average(uint8_t channel, uint8_t times)
{
    uint32_t sum = 0;
    uint8_t i;
    
    for (i = 0; i < times; i++) {
        sum += ADC1_ReadChannel(channel);
    }
    
    return (uint16_t)(sum / times);
}
