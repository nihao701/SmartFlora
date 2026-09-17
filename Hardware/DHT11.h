#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"
#include <stdint.h>

// DHT11 数据结构体
typedef struct {
    uint8_t humi_int;    // 湿度整数部分
    uint8_t humi_dec;    // 湿度小数部分（DHT11固定为0）
    uint8_t temp_int;    // 温度整数部分
    uint8_t temp_dec;    // 温度小数部分（DHT11固定为0）
    uint8_t checksum;    // 校验和
} DHT11_Data_t;

// 初始化 DHT11（引脚 PA2）
void DHT11_Init(void);

// 读取温湿度
// 返回值：1 = 成功，0 = 失败
uint8_t DHT11_Read(DHT11_Data_t *data);

#endif
