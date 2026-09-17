#include "DHT11.h"
#include "sysTick.h"

// ===== 引脚定义：PA2 =====
#define DHT11_PIN     GPIO_Pin_2
#define DHT11_PORT    GPIOA
#define DHT11_CLK     RCC_APB2Periph_GPIOA

/**
 * @brief  微秒级延时（粗略估算，72MHz 下 1us ≈ 8 个 NOP）
 * @note   用于 DHT11 时序控制，精度不要求极高
 */
static void DHT11_DelayUs(uint32_t us)
{
    uint32_t count = us * 8;
    while(count--) {
        __NOP();
    }
}

/**
 * @brief  将 DHT11 数据引脚切换为推挽输出
 */
static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = DHT11_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  将 DHT11 数据引脚切换为上拉输入
 */
static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = DHT11_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  读取 DHT11 数据引脚电平
 */
static uint8_t DHT11_ReadPin(void)
{
    return GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN);
}

/**
 * @brief  DHT11 初始化
 */
void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_CLK, ENABLE);
    DHT11_SetOutput();
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);   // 空闲时拉高
}

/**
 * @brief  读取一个字节（8位）
 * @note   DHT11 每一位都是“50us 低电平 + 高电平”，高电平持续时间决定 0/1
 *         高电平 26~28us → 0
 *         高电平 70us    → 1
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t i, byte = 0;
    
    for (i = 0; i < 8; i++) {
        // 等待 50us 低电平结束
        while (DHT11_ReadPin() == 0);
        
        // 延时 40us 后判断高电平是否还在
        DHT11_DelayUs(40);
        
        byte <<= 1;
        if (DHT11_ReadPin() == 1) {
            // 40us 后仍为高电平 → 数据位是 1
            byte |= 1;
            // 等待高电平结束
            while (DHT11_ReadPin() == 1);
        }
    }
    return byte;
}

/**
 * @brief  读取 DHT11 温湿度
 * @param  data: 数据结构体指针
 * @return 1 = 成功，0 = 失败
 * @note   时序：主机拉低 20ms → 主机拉高 30us → 切输入等响应 → 读 40 位
 */
uint8_t DHT11_Read(DHT11_Data_t *data)
{
    uint8_t buf[5];
    uint8_t i;
    uint32_t timeout;
    
    // 1. 主机发送起始信号
    DHT11_SetOutput();
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);   // 拉低
    DelayMs(20);                              // 至少 18ms
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);      // 拉高
    DHT11_DelayUs(30);                        // 20~40us
    
    // 2. 切换为输入模式，等待 DHT11 响应
    DHT11_SetInput();
    
    // 3. 等待 DHT11 拉低（80us 响应信号）
    timeout = 0;
    while (DHT11_ReadPin() == 1) {
        if (++timeout > 10000) return 0;   // 超时
    }
    
    // 4. 等待 DHT11 拉高（80us 响应结束）
    timeout = 0;
    while (DHT11_ReadPin() == 0) {
        if (++timeout > 10000) return 0;
    }
    timeout = 0;
    while (DHT11_ReadPin() == 1) {
        if (++timeout > 10000) return 0;
    }
    
    // 5. 读取 40 位数据（5 个字节）
    for (i = 0; i < 5; i++) {
        buf[i] = DHT11_ReadByte();
    }
    
    // 6. 校验：前 4 字节之和 == 第 5 字节
    if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4]) {
        data->humi_int = buf[0];
        data->humi_dec = buf[1];
        data->temp_int = buf[2];
        data->temp_dec = buf[3];
        data->checksum = buf[4];
        return 1;
    }
    
    return 0;
}
