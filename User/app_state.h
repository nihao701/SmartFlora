#ifndef __APP_STATE_H
#define __APP_STATE_H

#include <stdint.h>

// 系统状态枚举
typedef enum {
    STATE_MAIN = 0,     // 状态1：数据面板
    STATE_SELECT,       // 状态2：选择指标
    STATE_ADJUST,       // 状态3：调节阈值
    STATE_ALARM_ENV,    // 状态4：系统报警
} SystemState_t;

// 全局变量声明
extern SystemState_t g_state;
extern uint8_t g_selected;
extern uint8_t g_cursor;
extern uint8_t g_alarm_locked;

// 函数声明
void StateMachine_Init(void);
void StateMachine_Run(void);

#endif
