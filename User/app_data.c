#include "app_data.h"
#include "UART.h"
#include "bsp_pwm.h"
#include "buzzer.h"
#include "sysTick.h"
#include <stdio.h>

SensorData_t g_sensor;
Threshold_t g_threshold;
uint8_t g_sensor_ready = 0;

void Data_Init(void)
{
    // 默认阈值
    g_threshold.temp_min = 23.0f;
    g_threshold.temp_max = 30.0f;
    g_threshold.humi_min = 40.0f;
    g_threshold.humi_max = 80.0f;
    g_threshold.light_min = 1500;
    g_threshold.light_max = 3000;
    g_threshold.soil_min = 30;
    g_threshold.soil_max = 70;
}

// 判断单个指标的状态等级
static uint8_t JudgeStatus(float value, float min, float max, float buffer)
{
    if (value >= min && value <= max) {
        if (value >= min + buffer && value <= max - buffer) return 0;  // 良好
        return 1;  // 一级异常（接近边界）
    }
    if (value < min - buffer || value > max + buffer) return 2;  // 二级异常
    return 1;  // 一级异常（刚超出边界）
}

void Data_UpdateStatus(void)
{
    g_sensor.temp_status  = JudgeStatus(g_sensor.temp,  g_threshold.temp_min,  g_threshold.temp_max,  TEMP_BUFFER);
    g_sensor.humi_status  = JudgeStatus(g_sensor.humi,  g_threshold.humi_min,  g_threshold.humi_max,  HUMI_BUFFER);
    g_sensor.light_status = JudgeStatus(g_sensor.light, g_threshold.light_min, g_threshold.light_max, LIGHT_BUFFER);
    g_sensor.soil_status  = JudgeStatus(g_sensor.soil,  g_threshold.soil_min,  g_threshold.soil_max,  SOIL_BUFFER);
}

uint8_t Data_CountLevel2(void)
{
    uint8_t count = 0;
    if (g_sensor.temp_status  == 2) count++;
    if (g_sensor.humi_status  == 2) count++;
    if (g_sensor.light_status == 2) count++;
    if (g_sensor.soil_status  == 2) count++;
    return count;
}

// 上次状态记录
static uint8_t last_temp_status  = 0;
static uint8_t last_humi_status  = 0;
static uint8_t last_light_status = 0;
static uint8_t last_soil_status  = 0;

// 状态变化时的上报函数
void Data_ReportStatusChange(void)
{
    char buf[64];
    
    // 温度
    if (g_sensor.temp_status != last_temp_status) {
        if (g_sensor.temp_status == 0) {
            sprintf(buf, "[OK] Temp recovered: %d C\r\n", g_sensor.temp);
        } else if (g_sensor.temp_status == 1) {
            sprintf(buf, "[WARN] Temp abnormal: %d C\r\n", g_sensor.temp);
        } else {
            sprintf(buf, "[ALERT] Temp CRITICAL: %d C\r\n", g_sensor.temp);
        }
        UART1_SendString(buf);
        last_temp_status = g_sensor.temp_status;
    }
    
    // 湿度
    if (g_sensor.humi_status != last_humi_status) {
        if (g_sensor.humi_status == 0) {
            sprintf(buf, "[OK] Humi recovered: %d %%\r\n", g_sensor.humi);
        } else if (g_sensor.humi_status == 1) {
            sprintf(buf, "[WARN] Humi abnormal: %d %%\r\n", g_sensor.humi);
        } else {
            sprintf(buf, "[ALERT] Humi CRITICAL: %d %%\r\n", g_sensor.humi);
        }
        UART1_SendString(buf);
        last_humi_status = g_sensor.humi_status;
    }
    
    // 光照
    if (g_sensor.light_status != last_light_status) {
        if (g_sensor.light_status == 0) {
            sprintf(buf, "[OK] Light recovered: %d\r\n", g_sensor.light);
        } else if (g_sensor.light_status == 1) {
            sprintf(buf, "[WARN] Light abnormal: %d\r\n", g_sensor.light);
        } else {
            sprintf(buf, "[ALERT] Light CRITICAL: %d\r\n", g_sensor.light);
        }
        UART1_SendString(buf);
        last_light_status = g_sensor.light_status;
    }
    
    // 土壤
    if (g_sensor.soil_status != last_soil_status) {
        if (g_sensor.soil_status == 0) {
            sprintf(buf, "[OK] Soil recovered: %d %%\r\n", g_sensor.soil);
        } else if (g_sensor.soil_status == 1) {
            sprintf(buf, "[WARN] Soil abnormal: %d %%\r\n", g_sensor.soil);
        } else {
            sprintf(buf, "[ALERT] Soil CRITICAL: %d %%\r\n", g_sensor.soil);
        }
        UART1_SendString(buf);
        last_soil_status = g_sensor.soil_status;
    }
}

// 调节阈值：para=指标（0~3），target=0最大值/1最小值，delta=+1/-1
void AdjustThreshold(uint8_t para, uint8_t target, int8_t delta)
{
    switch (para) {
        case 0:   // 温度
            if (target == 0) {
                g_threshold.temp_max += delta;
                if (g_threshold.temp_max < g_threshold.temp_min + 1)
                    g_threshold.temp_max = g_threshold.temp_min + 1;
            } else {
                g_threshold.temp_min += delta;
                if (g_threshold.temp_min > g_threshold.temp_max - 1)
                    g_threshold.temp_min = g_threshold.temp_max - 1;
            }
            break;
        case 1:   // 湿度
            if (target == 0) {
                g_threshold.humi_max += delta;
                if (g_threshold.humi_max < g_threshold.humi_min + 1)
                    g_threshold.humi_max = g_threshold.humi_min + 1;
            } else {
                g_threshold.humi_min += delta;
                if (g_threshold.humi_min > g_threshold.humi_max - 1)
                    g_threshold.humi_min = g_threshold.humi_max - 1;
            }
            break;
        case 2:   // 光照（步进 10）
            if (target == 0) {
                g_threshold.light_max += delta * 10;
                if (g_threshold.light_max < g_threshold.light_min + 10)
                    g_threshold.light_max = g_threshold.light_min + 10;
            } else {
                g_threshold.light_min += delta * 10;
                if (g_threshold.light_min > g_threshold.light_max - 10)
                    g_threshold.light_min = g_threshold.light_max - 10;
            }
            break;
        case 3:   // 土壤
            if (target == 0) {
                g_threshold.soil_max += delta;
                if (g_threshold.soil_max < g_threshold.soil_min + 1)
                    g_threshold.soil_max = g_threshold.soil_min + 1;
            } else {
                g_threshold.soil_min += delta;
                if (g_threshold.soil_min > g_threshold.soil_max - 1)
                    g_threshold.soil_min = g_threshold.soil_max - 1;
            }
            break;
    }
}

// ===== 光照闭环控制 =====
void LightControl(void)
{
    static int16_t duty = 0;
    
    // 光照状态良好时，慢慢降
    if (g_sensor.light_status == 0) {
        if (duty > 0) duty -= 1;
        PWM_SetDuty((uint8_t)duty);
        return;
    }
    
    if (g_sensor.light == 0) return;
    
    int16_t adc = (int16_t)g_sensor.light;
    int16_t adc_min = (int16_t)g_threshold.light_min;
    int16_t adc_max = (int16_t)g_threshold.light_max;
    
    if (adc > adc_max) {
        duty += 5;        // 环境太暗 → 快速增亮
    } else if (adc < adc_min) {
        duty -= 5;        // 环境太亮 → 快速降亮
    } else {
        if (duty > 0) duty -= 1;   // 合适范围，缓慢归零
    }
    
    if (duty > 100) duty = 100;
    if (duty < 0)   duty = 0;
    
    PWM_SetDuty((uint8_t)duty);
}

// ===== 执行器处理（蜂鸣器 + 补光） =====
void Data_ActuatorHandler(void)
{
    static uint32_t last_light = 0;
    
    if (GetSysTick() - last_light >= 500) {
        last_light = GetSysTick();
        LightControl();
    }
}
