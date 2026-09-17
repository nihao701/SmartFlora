#include "sysTick.h"

static volatile uint32_t sys_tick_ms = 0;

void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while(1);
    }
    NVIC_SetPriority(SysTick_IRQn, 15);
}

uint32_t GetSysTick(void)
{
    return sys_tick_ms;
}

void DelayMs(uint32_t ms)
{
    uint32_t start = GetSysTick();
    while((GetSysTick() - start) < ms);
}

void DelayUs(uint32_t us)
{
    uint32_t count = us * 8;
    while(count--) {
        __NOP();
    }
}

// ========== 提供给中断调用的函数 ==========
void SysTick_Handler_Callback(void)
{
    sys_tick_ms++;
}
