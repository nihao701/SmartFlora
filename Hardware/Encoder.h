#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"
#include <stdint.h>

typedef enum {
    ENC_EVENT_NONE = 0,
    ENC_EVENT_CW,
    ENC_EVENT_CCW,
} EncoderEvent_t;

void Encoder_Init(void);
EncoderEvent_t Encoder_GetEvent(void);
int16_t Encoder_GetCount(void);
void Encoder_ResetCount(void);

#endif
