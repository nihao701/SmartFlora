#include "app_state.h"
#include "app_data.h"
#include "KEY.h"
#include "Encoder.h"
#include "OLED.h"
#include "UART.h"
#include "sysTick.h"
#include "bsp_flash.h"
#include "bsp_pwm.h"
#include "buzzer.h"
#include "ir.h"
#include "servo.h"
#include <stdio.h>
#include <string.h>

// ===== 全局变量定义 =====
SystemState_t g_state = STATE_MAIN;
uint8_t g_selected = 0;
uint8_t g_cursor = 0;
uint8_t g_alarm_locked = 0;   // 0=未锁定，1=已锁定
static uint8_t g_system_ready = 0;    
static uint32_t g_boot_time = 0; 
uint8_t g_door_locked = 0;      // 0=未锁定，1=已锁定
uint16_t g_person_count = 0;    // 锁定期间检测到的人数

// ===== 函数前置声明 =====
static void State_Main(void);
static void State_Select(void);
static void State_Adjust(void);
static void DrawMainScreen(void);
static void DrawSelectScreen(void);
static void DrawAdjustScreen(void);
static void DrawAlarmScreen(void);
static void State_AlarmEnv(void);
static void State_Door(void);
static void State_AlarmSec(void);

// ===== 初始化 =====
void StateMachine_Init(void)
{
    g_state = STATE_MAIN;
    g_selected = 0;
    g_cursor = 0;
    g_alarm_locked = 0;
	g_boot_time = GetSysTick();    
    g_system_ready = 0;
}

// ===== 红外控制 =====
void IR_Task(void)
{
    static uint8_t ir_triggered = 0;
    
    // 开机 20 秒后才允许红外触发
    if (GetSysTick() - g_boot_time >= 20000) {
        g_system_ready = 1;
    }
    
    if (!g_system_ready) return;
    
    if (IR_Detected() && !ir_triggered) {
        ir_triggered = 1;
        
        if (g_door_locked) {
            g_person_count++;
            char buf[64];
            sprintf(buf, "[PERSON] Person detected! Count: %d\r\n", g_person_count);
            UART1_SendString(buf);
            g_state = STATE_ALARM_SEC;
        } else {
            g_state = STATE_DOOR;
        }
    }
    
    if (!IR_Detected()) {
        ir_triggered = 0;
    }
}

// ===== 蜂鸣器控制 =====
void Buzzer_Task(void)
{
    static uint32_t last_toggle = 0;
    static uint8_t buzzer_state = 0;
    uint8_t level2_count = Data_CountLevel2();
    
    if (g_state == STATE_ALARM_ENV || g_state == STATE_ALARM_SEC) {
        Buzzer_On();
        buzzer_state = 1;
        last_toggle = GetSysTick();
    } else if (level2_count >= 1) {
        if (GetSysTick() - last_toggle >= 500) {
            last_toggle = GetSysTick();
            buzzer_state = !buzzer_state;
        }
        if (buzzer_state) Buzzer_On();
        else Buzzer_Off();
    } else {
        Buzzer_Off();
        buzzer_state = 0;
    }
}

// ===== OLED调度 =====
void OLED_Task(void)
{
    static uint32_t last_draw = 0;
	uint32_t interval = 500;
	
	// 状态2/3 刷新更快
    if (g_state == STATE_SELECT || g_state == STATE_ADJUST) {
        interval = 100;
    }
	
    if (GetSysTick() - last_draw >= interval) {
        last_draw = GetSysTick();
        
        switch (g_state) {
            case STATE_MAIN:
                DrawMainScreen();
                break;
            case STATE_DOOR:
                // 第1行显示 "Door Open!"，第2~4行显示数据
                OLED_ShowString(1, 1, "Door Open!      ");
                OLED_ShowString(2, 1, "T:");
                OLED_ShowNum(2, 3, (uint16_t)g_sensor.temp, 2);
                OLED_ShowString(2, 5, "C H:");
                OLED_ShowNum(2, 9, (uint16_t)g_sensor.humi, 2);
                OLED_ShowString(2, 12, "%");
                OLED_ShowString(3, 1, "L:");
                OLED_ShowNum(3, 3, g_sensor.light, 4);
                OLED_ShowString(3, 8, "S:");
                OLED_ShowNum(3, 10, g_sensor.soil, 3);
                OLED_ShowString(3, 13, "%");
                break;
            case STATE_SELECT:
                DrawSelectScreen();
                break;
            case STATE_ADJUST:
                DrawAdjustScreen();
                break;
            case STATE_ALARM_ENV:
                DrawAlarmScreen();
                break;
            case STATE_ALARM_SEC:
                OLED_ShowString(1, 1, "!! SECURITY !!");
                OLED_ShowString(2, 1, "Person Detected!");
                OLED_ShowString(3, 1, "Door Locked");
                break;
            default:
                DrawMainScreen();
                break;
        }
    }
}

// ===== 主调度 =====
void StateMachine_Run(void)
{
    // 自动进入状态4（只要没锁定）
    if (g_sensor_ready && 
        g_state != STATE_ALARM_ENV && 
        Data_CountLevel2() >= 3 && 
        !g_alarm_locked) 
    {
        g_state = STATE_ALARM_ENV;
    }
    
    switch (g_state) {
        case STATE_MAIN:      State_Main();      break;
        case STATE_SELECT:    State_Select();    break;
        case STATE_ADJUST:    State_Adjust();    break;
        case STATE_ALARM_ENV: State_AlarmEnv();  break;
		case STATE_DOOR:	  State_Door();      break;
		case STATE_ALARM_SEC: State_AlarmSec();	 break;
        default:              g_state = STATE_MAIN; break;
    }
}

// ===== 显示状态1（数据面板） =====
static void DrawMainScreen(void)
{
    // 第1行：标题
    OLED_ShowString(1, 1, "Smart Flower");
    
    // 第2行：温度 + 湿度
    OLED_ShowString(2, 1, "T:");
    OLED_ShowNum(2, 3, (uint16_t)g_sensor.temp, 2);
    OLED_ShowString(2, 5, "C H:");
    OLED_ShowNum(2, 9, (uint16_t)g_sensor.humi, 2);
    OLED_ShowString(2, 12, "%");
    
    // 第3行：光照 + 土壤
    OLED_ShowString(3, 1, "L:");
    OLED_ShowNum(3, 3, g_sensor.light, 4);
    OLED_ShowString(3, 8, "S:");
    OLED_ShowNum(3, 10, g_sensor.soil, 3);
    OLED_ShowString(3, 13, "%");
    
    // 第4行：左边状态，右边 PWM
    if (g_sensor.temp_status == 0 && g_sensor.humi_status == 0 &&
        g_sensor.light_status == 0 && g_sensor.soil_status == 0) {
        OLED_ShowString(4, 1, "OK   ");
    } else if (g_sensor.temp_status == 2 || g_sensor.humi_status == 2 ||
               g_sensor.light_status == 2 || g_sensor.soil_status == 2) {
        OLED_ShowString(4, 1, "ALERT");
    } else {
        OLED_ShowString(4, 1, "WARN ");
    }
    
    OLED_ShowString(4, 7, "PWM:");
    OLED_ShowNum(4, 11, PWM_GetDuty(), 3);
    OLED_ShowString(4, 14, "%");
}

// ===== 状态1：数据面板 =====
static void State_Main(void)
{
    static uint32_t last_draw = 0;
    
    // 如果门锁定，按键失效
    if (!g_door_locked)
    {
        // 正常状态1 逻辑
        if (GetSysTick() - last_draw >= 500) {
            last_draw = GetSysTick();
            DrawMainScreen();
        }
        
        // KEY1 短按 → 状态2
        if (KEY1_GetEvent() == KEY_EVENT_SHORT) {
            g_state = STATE_SELECT;
            g_selected = 0;
            OLED_Clear();
        }
        
        // KEY2 长按 → 解锁
        if (g_alarm_locked && KEY2_GetEvent() == KEY_EVENT_LONG) {
            g_alarm_locked = 0;
            // 删掉 unlock_tip_flag / unlock_tip_time
            // 删掉 OLED_ShowString("Unlocked!")
            // 删掉 return
        }
        
        // 异常全恢复 → 自动解锁
        if (Data_CountLevel2() == 0) {
            g_alarm_locked = 0;
        }
    }
}

// ===== 显示状态2（选择指标） =====
static void DrawSelectScreen(void)
{
    const char *names[4] = {"Temp ", "Humi ", "Light", "Soil "};
    uint8_t i;
    
    for (i = 0; i < 4; i++) {
        if (i == g_selected) {
            OLED_ShowString(i + 1, 1, ">");
        } else {
            OLED_ShowString(i + 1, 1, " ");
        }
        OLED_ShowString(i + 1, 2, (char *)names[i]);
    }
}

// ===== 状态2：选择指标 =====
static void State_Select(void)
{
    static uint32_t last_draw = 0;
    if (GetSysTick() - last_draw >= 200) {
        last_draw = GetSysTick();
        DrawSelectScreen();
    }
    
    // KEY1 短按：切换指标
    KeyEvent_t evt1 = KEY1_GetEvent();
    if (evt1 == KEY_EVENT_SHORT) {
        g_selected = (g_selected + 1) % 4;
        DrawSelectScreen();
    } else if (evt1 == KEY_EVENT_LONG) {
        // 进入状态3
        g_state = STATE_ADJUST;
        g_cursor = 0;
        OLED_Clear();
    }
    
    // KEY2 短按：返回状态1
    KeyEvent_t evt2 = KEY2_GetEvent();
    if (evt2 == KEY_EVENT_SHORT) {
        g_state = STATE_MAIN;
        OLED_Clear();
    }
}

// ===== 显示状态3（调节阈值） =====
static void DrawAdjustScreen(void)
{
    char buf[20];
    
    const char *names[4] = {"Temp ", "Humi ", "Light", "Soil "};
    OLED_ShowString(1, 1, "Set ");
    OLED_ShowString(1, 5, (char *)names[g_selected]);
    
    // 最大值
    if (g_cursor == 0) {
        OLED_ShowString(2, 1, ">Max:");
    } else {
        OLED_ShowString(2, 1, " Max:");
    }
    switch (g_selected) {
        case 0: sprintf(buf, "%d C  ", (int)g_threshold.temp_max); break;
        case 1: sprintf(buf, "%d %%  ", (int)g_threshold.humi_max); break;
        case 2: sprintf(buf, "%d    ", g_threshold.light_max); break;
        case 3: sprintf(buf, "%d %%  ", g_threshold.soil_max); break;
    }
    OLED_ShowString(2, 6, buf);
    
    // 最小值
    if (g_cursor == 1) {
        OLED_ShowString(3, 1, ">Min:");
    } else {
        OLED_ShowString(3, 1, " Min:");
    }
    switch (g_selected) {
        case 0: sprintf(buf, "%d C  ", (int)g_threshold.temp_min); break;
        case 1: sprintf(buf, "%d %%  ", (int)g_threshold.humi_min); break;
        case 2: sprintf(buf, "%d    ", g_threshold.light_min); break;
        case 3: sprintf(buf, "%d %%  ", g_threshold.soil_min); break;
    }
    OLED_ShowString(3, 6, buf);
    
    // 第4行：提示
    OLED_ShowString(4, 1, "K1:Save K2:Back");
}

// ===== 状态3：调节阈值 =====
static void State_Adjust(void)
{
    static uint32_t last_draw = 0;
    static uint32_t save_tip_time = 0;
    static uint8_t  save_tip_flag = 0;
    
    // 保存提示阶段
    if (save_tip_flag) {
        if (GetSysTick() - save_tip_time < 1200) {
            return;
        }
        save_tip_flag = 0;
        OLED_Clear();
        DrawAdjustScreen();
        return;
    }
    
    // 正常状态3显示
    if (GetSysTick() - last_draw >= 100) {
        last_draw = GetSysTick();
        DrawAdjustScreen();
    }
    
    // 编码器旋转调节数值
    EncoderEvent_t enc = Encoder_GetEvent();
    if (enc == ENC_EVENT_CW) {
        AdjustThreshold(g_selected, g_cursor, +1);
    } else if (enc == ENC_EVENT_CCW) {
        AdjustThreshold(g_selected, g_cursor, -1);
    }
    
    // KEY1 短按：切换最大值/最小值
    KeyEvent_t evt1 = KEY1_GetEvent();
    if (evt1 == KEY_EVENT_SHORT) {
        g_cursor = (g_cursor + 1) % 2;
    } else if (evt1 == KEY_EVENT_LONG) {
        Flash_SaveThreshold();
        
        OLED_Clear();
        OLED_ShowString(2, 3, "Saved!");
        OLED_ShowString(3, 3, "OK");
        
        save_tip_flag = 1;
        save_tip_time = GetSysTick();
        return;
    }
    
    // KEY2 短按：返回状态2
    KeyEvent_t evt2 = KEY2_GetEvent();
    if (evt2 == KEY_EVENT_SHORT) {
        g_state = STATE_SELECT;
        OLED_Clear();
    }
}

// ===== 显示状态4（系统报警） =====
static void DrawAlarmScreen(void)
{
    OLED_ShowString(1, 1, "!! SYSTEM ALERT!");
    OLED_ShowString(2, 1, "MULTI-SENSOR FLT");
    
    if (Data_CountLevel2() >= 4) {
        OLED_ShowString(3, 1, "ALL");
    } else {
        char line[20];
        int len;
        line[0] = '\0';
        if (g_sensor.temp_status == 2)  strcat(line, "TEMP ");
        if (g_sensor.humi_status == 2)  strcat(line, "HUMI ");
        if (g_sensor.light_status == 2) strcat(line, "LIGHT ");
        if (g_sensor.soil_status == 2)  strcat(line, "SOIL ");
        len = strlen(line);
        if (len > 0) line[len - 1] = '\0';
        OLED_ShowString(3, 1, line);
    }
    
    OLED_ShowString(4, 1, "CHECK DEVICE!");
}

// ===== 状态4：系统报警 =====
static void State_AlarmEnv(void)
{
    static uint32_t last_draw = 0;
    static uint32_t last_report = 0;
    
    if (GetSysTick() - last_draw >= 500) {
        last_draw = GetSysTick();
        DrawAlarmScreen();
    }
    
    if (GetSysTick() - last_report >= 2000) {
        last_report = GetSysTick();
        char buf[64];
        sprintf(buf, "[CRITICAL] System Alert: %d sensors in level-2 alarm!\r\n", Data_CountLevel2());
        UART1_SendString(buf);
    }
    
    // 只有 KEY2 长按才退出（锁定）
    if (KEY2_GetEvent() == KEY_EVENT_LONG) {
        OLED_Clear();
        g_state = STATE_MAIN;
        g_alarm_locked = 1;
    }
}

// 状态6 处理函数
static void State_Door(void)
{
    static uint32_t last_detect_time = 0;
    static uint8_t first_enter = 1;
    
    if (first_enter) {
        first_enter = 0;
        Servo_SetAngle(90);
        last_detect_time = GetSysTick();
    }
    
    if (IR_Detected()) {
        last_detect_time = GetSysTick();
    }
    
    // 人离开后 3 秒关门
    if (GetSysTick() - last_detect_time >= 3000) {
        Servo_SetAngle(0);
        first_enter = 1;
        g_state = STATE_MAIN;
    }
}

static void State_AlarmSec(void)
{
    static uint32_t alarm_time = 0;
    static uint8_t first_enter = 1;
    
    if (first_enter) {
        first_enter = 0;
        alarm_time = GetSysTick();
    }
    
    // 报警 3 秒后回状态1（门保持锁定）
    if (GetSysTick() - alarm_time >= 3000) {
        first_enter = 1;
        g_state = STATE_MAIN;
    }
}
