#include "stm32f10x.h"

static uint32_t SysTimeMs = 0;     // 系统时间计数(ms)

/**
 * @brief 微秒级延时
 * @param xus 延时微秒数
 * @retval 无
 */
void Delay_us(uint32_t xus)
{
    SysTick->LOAD = 72 * xus;      // 设置定时器重装值（72MHz主频）
    SysTick->VAL = 0x00;            // 清空当前计数值
    SysTick->CTRL = 0x00000005;     // 设置时钟源为HCLK，启动定时器
    while(!(SysTick->CTRL & 0x00010000));  // 等待计数到0
    SysTick->CTRL = 0x00000004;     // 关闭定时器
}

/**
 * @brief 毫秒级延时
 * @param xms 延时毫秒数
 * @retval 无
 */
void Delay_ms(uint32_t xms)
{
    while(xms--)
    {
        Delay_us(1000);              // 1ms = 1000us
    }
}

/**
 * @brief 秒级延时
 * @param xs 延时秒数
 * @retval 无
 */
void Delay_s(uint32_t xs)
{
    while(xs--)
    {
        Delay_ms(1000);              // 1s = 1000ms
    }
}

/**
 * @brief SysTick中断处理函数（使用弱定义）
 * @param 无
 * @retval 无
 */
__attribute__((weak)) void SysTick_Handler(void)
{
    SysTimeMs++;                     // 每1ms增加一次
}
/**
 * @brief 初始化系统时间
 * @param 无
 * @retval 无
 */
void SysTime_Init(void)
{
    // 配置SysTick每1ms中断一次 (SystemCoreClock=72MHz)
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while(1);                    // 配置失败，死循环
    }
}

/**
 * @brief 获取系统运行时间
 * @param 无
 * @retval 系统运行毫秒数
 */
uint32_t GetSysTimeMs(void)
{
    return SysTimeMs;
}
