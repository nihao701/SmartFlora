#ifndef __APP_DATA_H
#define __APP_DATA_H

#include <stdint.h>

// ===== 阈值结构体 =====
typedef struct {
    float temp_min, temp_max;
    float humi_min, humi_max;
    uint16_t light_min, light_max;
    uint8_t soil_min, soil_max;
} Threshold_t;

// ===== 传感器数据 + 状态等级 =====
typedef struct {
    uint8_t temp;          // 温度（整数）
    uint8_t humi;          // 湿度（整数）
    uint16_t light;        // 光照 ADC
    uint8_t soil;          // 土壤湿度百分比
    
    uint8_t temp_status;   // 0=良好, 1=一级异常, 2=二级异常
    uint8_t humi_status;
    uint8_t light_status;
    uint8_t soil_status;
} SensorData_t;

// ===== 全局变量声明 =====
extern SensorData_t g_sensor;
extern Threshold_t g_threshold;
extern uint8_t g_sensor_ready;   // 传感器数据是否就绪

// ===== 阈值分级缓冲宏 =====
#define TEMP_BUFFER     0.5f
#define HUMI_BUFFER     3.0f
#define LIGHT_BUFFER    100
#define SOIL_BUFFER     3

// ===== 函数声明 =====
void Data_Init(void);
void Data_UpdateStatus(void);
uint8_t Data_CountLevel2(void);
void Data_ReportStatusChange(void);
void Data_ActuatorHandler(void);
void AdjustThreshold(uint8_t para, uint8_t target, int8_t delta);
void LightControl(void);

#endif
