#include "KEY.h"
#include "sysTick.h"

// 按键状态机状态
typedef enum {
    KEY_STATE_IDLE,      // 空闲：等待按下
    KEY_STATE_DEBOUNCE,  // 去抖：刚检测到按下，等 20ms
    KEY_STATE_PRESSED,   // 已按下：确认有效，等待释放或长按
} KeyState_t;

// ===== KEY1 状态机变量 =====
static KeyState_t key1_state = KEY_STATE_IDLE;
static uint32_t key1_press_time = 0;   // 记录按下时刻
static uint8_t key1_long_flag = 0;     // 长按触发标志

// ===== KEY2 状态机变量 =====
static KeyState_t key2_state = KEY_STATE_IDLE;
static uint32_t key2_press_time = 0;
static uint8_t key2_long_flag = 0;

/**
 * @brief  初始化 KEY1（PB1）和 KEY2（PB12）
 * @note   内部上拉输入，按下读到低电平
 */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能 GPIOB 时钟
    RCC_APB2PeriphClockCmd(KEY1_CLK, ENABLE);
    
    // KEY1 PB1 上拉输入
    GPIO_InitStruct.GPIO_Pin = KEY1_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &GPIO_InitStruct);
    
    // KEY2 PB12 上拉输入
    GPIO_InitStruct.GPIO_Pin = KEY2_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY2_PORT, &GPIO_InitStruct);
}

/**
 * @brief  KEY1 事件获取（状态机 + 非阻塞消抖）
 * @return KEY_EVENT_NONE / SHORT / LONG
 * @note   每次主循环调用一次，不阻塞
 */
KeyEvent_t KEY1_GetEvent(void)
{
    // 读取当前电平：按下 = 低电平（Bit_RESET）
    uint8_t is_pressed = (GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == Bit_RESET);
    uint32_t now = GetSysTick();
    KeyEvent_t event = KEY_EVENT_NONE;
    
    switch (key1_state) {
        // ===== 状态1：空闲 =====
        case KEY_STATE_IDLE:
            if (is_pressed) {
                // 检测到按下，进入去抖状态
                key1_state = KEY_STATE_DEBOUNCE;
                key1_press_time = now;
                key1_long_flag = 0;
            }
            break;
        
        // ===== 状态2：去抖 =====
        case KEY_STATE_DEBOUNCE:
            if (is_pressed) {
                // 持续 20ms 仍按下，视为有效按下
                if (now - key1_press_time >= 20) {
                    key1_state = KEY_STATE_PRESSED;
                    key1_press_time = now;   // 重置计时，用于长按判断
                    key1_long_flag = 0;
                }
            } else {
                // 去抖期间释放，视为抖动，回到空闲
                key1_state = KEY_STATE_IDLE;
            }
            break;
        
        // ===== 状态3：已按下 =====
        case KEY_STATE_PRESSED:
            if (is_pressed) {
                // 持续按住 1000ms 触发长按
                if (now - key1_press_time >= 1000) {
                    event = KEY_EVENT_LONG;
                    key1_long_flag = 1;      // 标记已触发长按
                    key1_press_time = now;   // 重置，防止连续触发
                }
            } else {
                // 按键释放
                // 只有“没触发过长按”且“按下时间 <1 秒”才视为短按
                if (!key1_long_flag && (now - key1_press_time < 1000)) {
                    event = KEY_EVENT_SHORT;
                }
                key1_state = KEY_STATE_IDLE;
                key1_long_flag = 0;
            }
            break;
        
        default:
            key1_state = KEY_STATE_IDLE;
            break;
    }
    return event;
}

/**
 * @brief  KEY2 事件获取（逻辑与 KEY1 完全相同，只是引脚不同）
 */
KeyEvent_t KEY2_GetEvent(void)
{
    uint8_t is_pressed = (GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == Bit_RESET);
    uint32_t now = GetSysTick();
    KeyEvent_t event = KEY_EVENT_NONE;
    
    switch (key2_state) {
        case KEY_STATE_IDLE:
            if (is_pressed) {
                key2_state = KEY_STATE_DEBOUNCE;
                key2_press_time = now;
                key2_long_flag = 0;
            }
            break;
        case KEY_STATE_DEBOUNCE:
            if (is_pressed) {
                if (now - key2_press_time >= 20) {
                    key2_state = KEY_STATE_PRESSED;
                    key2_press_time = now;
                    key2_long_flag = 0;
                }
            } else {
                key2_state = KEY_STATE_IDLE;
            }
            break;
        case KEY_STATE_PRESSED:
            if (is_pressed) {
                if (now - key2_press_time >= 1000) {
                    event = KEY_EVENT_LONG;
                    key2_long_flag = 1;
                    key2_press_time = now;
                }
            } else {
                if (!key2_long_flag && (now - key2_press_time < 1000)) {
                    event = KEY_EVENT_SHORT;
                }
                key2_state = KEY_STATE_IDLE;
                key2_long_flag = 0;
            }
            break;
        default:
            key2_state = KEY_STATE_IDLE;
            break;
    }
    return event;
}
