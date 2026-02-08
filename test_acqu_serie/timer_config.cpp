#include "timer_config.h"
#include "protocol.h"

const uint32_t TIMER_BASE_FREQ_HZ = 1000000;

uint32_t TIMER_ISR_FREQ_HZ;
uint32_t TIMER_ISR_TICKS; // nb de ticks à attendre avant de relancer une isr (onTimer())

const uint8_t PIN_LED = 2;
bool ledState = true;
hw_timer_t* timer = nullptr;
volatile uint32_t timerTicks = 0;

void isrBegin() {
    TIMER_ISR_FREQ_HZ  = freqEch << 2;
    TIMER_ISR_TICKS    = TIMER_BASE_FREQ_HZ / TIMER_ISR_FREQ_HZ; // nb de ticks à attendre avant de relancer une isr (onTimer())

    timer = timerBegin(TIMER_BASE_FREQ_HZ);
    timerAttachInterrupt(timer, &onTimer);
    timerWrite(timer, 0);
    timerAlarm(timer, TIMER_ISR_TICKS, true, 0);
    Serial.println("Timer intialisés pour fe =");
    Serial.println(freqEch);
}

void IRAM_ATTR onTimer() {
    timerTicks += 1;
}

void blink(int nb, uint8_t t_ms){
    ledState = false;
    digitalWrite(PIN_LED, ledState);
    for (int i = 0; i < 2*nb; i++){
      digitalWrite(PIN_LED, ledState);
      ledState = !ledState;
      delay(t_ms);
    }
    ledState = false;
    digitalWrite(PIN_LED, ledState);
}
