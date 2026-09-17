#include "LightSensor.h"
#include "Delay.h"


#define LIGHT_SENSOR_PIN   GPIO_Pin_13  // 接在PB13
#define LIGHT_SENSOR_PORT  GPIOB
#define LIGHT_SENSOR_CLOCK RCC_APB2Periph_GPIOB

/**
 * @brief 初始化光敏传感器
 * @param 无
 * @retval 无
 */
void LightSensor_Init(void)
{
    RCC_APB2PeriphClockCmd(LIGHT_SENSOR_CLOCK, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LIGHT_SENSOR_PIN;
    GPIO_Init(LIGHT_SENSOR_PORT, &GPIO_InitStructure);
}

/**
 * @brief 读取光敏传感器状态
 * @param 无
 * @retval 0-有光(Bright) 1-无光(Dark)
 */
uint8_t LightSensor_Read(void)
{
    // 读取引脚状态
    // 如果模块输出低电平表示有光，则直接返回读取值
    // 如果模块输出高电平表示有光，则需要取反
    return GPIO_ReadInputDataBit(LIGHT_SENSOR_PORT, LIGHT_SENSOR_PIN);
}

