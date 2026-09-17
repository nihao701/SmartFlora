#include "app_uart_protocol.h"
#include "UART.h"
#include "app_data.h"
#include "app_state.h"
#include "actuator.h"
#include "servo.h"
#include "sysTick.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t g_control_mode = 0;

uint8_t UART_GetControlMode(void)
{
    return g_control_mode;
}

void UART_Protocol_Handler(void)
{
    char cmd[16];
    int a, b;
    char buf[64];
    
    if (!rx_complete) return;
    
    // ===== GET =====
    if (strcmp((char*)rx_buffer, "GET") == 0) {
        sprintf(buf, "DATA T=%d H=%d L=%d S=%d\r\n",
                g_sensor.temp, g_sensor.humi, g_sensor.light, g_sensor.soil);
        UART1_SendString(buf);
    }
    // ===== SET =====
    else if (sscanf((char*)rx_buffer, "SET %s %d %d", cmd, &a, &b) == 3) {
        if (strcmp(cmd, "T") == 0) {
            g_threshold.temp_min = a;
            g_threshold.temp_max = b;
        } else if (strcmp(cmd, "H") == 0) {
            g_threshold.humi_min = a;
            g_threshold.humi_max = b;
        } else if (strcmp(cmd, "L") == 0) {
            g_threshold.light_min = a;
            g_threshold.light_max = b;
        } else if (strcmp(cmd, "S") == 0) {
            g_threshold.soil_min = a;
            g_threshold.soil_max = b;
        } else {
            UART1_SendString("ERROR\r\n");
            rx_complete = 0;
            return;
        }
        UART1_SendString("OK\r\n");
    }
    // ===== CTRL =====
    else if (strncmp((char*)rx_buffer, "CTRL", 4) == 0) {
        char dev[8], state[8];
        if (sscanf((char*)rx_buffer, "CTRL %s %s", dev, state) == 2) {
            if (strcmp(dev, "FAN") == 0) {
                if (strcmp(state, "ON") == 0)  Fan_On();
                else                           Fan_Off();
            } else if (strcmp(dev, "HUM") == 0) {
                if (strcmp(state, "ON") == 0)  Humidifier_On();
                else                           Humidifier_Off();
            } else if (strcmp(dev, "PUMP") == 0) {
                if (strcmp(state, "ON") == 0)  Pump_On();
                else                           Pump_Off();
            } else {
                UART1_SendString("ERROR\r\n");
                rx_complete = 0;
                return;
            }
            UART1_SendString("OK\r\n");
        } else {
            UART1_SendString("ERROR\r\n");
        }
    }
    // ===== MODE =====
    else if (strcmp((char*)rx_buffer, "MODE AUTO") == 0) {
        g_control_mode = 0;
        UART1_SendString("OK\r\n");
    } else if (strcmp((char*)rx_buffer, "MODE MANUAL") == 0) {
        g_control_mode = 1;
        UART1_SendString("OK\r\n");
    }
    // ===== DOOR =====
    else if (strcmp((char*)rx_buffer, "DOOR OPEN") == 0) {
        Servo_SetAngle(90);
        UART1_SendString("OK\r\n");
    } else if (strcmp((char*)rx_buffer, "DOOR CLOSE") == 0) {
        Servo_SetAngle(0);
        UART1_SendString("OK\r\n");
    }
    // ===== ALARM CLEAR =====
    else if (strcmp((char*)rx_buffer, "ALARM CLEAR") == 0) {
        g_alarm_locked = 0;
        UART1_SendString("OK\r\n");
    }
    // ===== Î´ÖªÖ¸Áî =====
    else {
        UART1_SendString("ERROR\r\n");
    }
    
    rx_complete = 0;
}
