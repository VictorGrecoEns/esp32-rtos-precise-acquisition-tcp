#include "timer_config.h"

const uint32_t TIMER_BASE_FREQ_HZ = 1000000;
const uint32_t TIMER_ISR_FREQ_HZ  = 800;
const uint32_t TIMER_ISR_TICKS   = TIMER_BASE_FREQ_HZ / TIMER_ISR_FREQ_HZ;

const uint8_t PIN_LED = 2;
bool ledState = true;
hw_timer_t* timer = nullptr;
volatile uint32_t timerTicks = 0;

void isrBegin() {
    timer = timerBegin(TIMER_BASE_FREQ_HZ);
    timerAttachInterrupt(timer, &onTimer);
    timerWrite(timer, 0);
    timerAlarm(timer, TIMER_ISR_TICKS, true, 0);
    Serial.println("Timer intialisés");
}

void IRAM_ATTR onTimer() {
    timerTicks += 1;
}
