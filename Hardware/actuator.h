#ifndef __ACTUATOR_H
#define __ACTUATOR_H

#include "stm32f10x.h"
#include <stdint.h>

void Actuator_Init(void);

// 各执行器控制
void Fan_On(void);      void Fan_Off(void);
void Humidifier_On(void); void Humidifier_Off(void);
void Pump_On(void);     void Pump_Off(void);

// 自动控制（在 SensorTask 里调用）
void AutoControl(void);

#endif
