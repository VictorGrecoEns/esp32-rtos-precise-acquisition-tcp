#pragma once
#include <Arduino.h>

void isrBegin();
void onTimer();

extern hw_timer_t* timer;
extern volatile uint32_t timerTicks;

extern const uint32_t TIMER_BASE_FREQ_HZ;
extern const uint32_t TIMER_ISR_FREQ_HZ;
extern const uint32_t TIMER_ISR_TICKS;

extern const uint8_t PIN_LED;
extern bool ledState;
