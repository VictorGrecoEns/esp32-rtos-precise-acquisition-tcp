#include "acquisition.h"
#include "storage.h"
#include "config.h"

#include <Arduino.h>

// ===== Timer =====
static hw_timer_t* timer = NULL;
static const uint32_t TIMER_BASE_FREQ_HZ = 1000000;
static uint32_t TIMER_ISR_FREQ_HZ;
static uint32_t TIMER_ISR_TICKS; // nb de ticks à attendre avant de relancer une isr (onTimer())
static uint32_t timerTicks = 0;

// ===== Oversampling =====
static uint32_t oversampleAcc[NB_CAPTEURS] = {0};
static uint8_t  oversampleCount = 0;

void onTimer();

void initAcquisition()
{
    TIMER_ISR_FREQ_HZ  = SAMPLE_FREQUENCY * OVERSAMPLING; 
    TIMER_ISR_TICKS    = TIMER_BASE_FREQ_HZ / TIMER_ISR_FREQ_HZ; // nb de ticks à attendre avant de relancer une isr (onTimer())
    
    timer = timerBegin(TIMER_BASE_FREQ_HZ);
    timerAttachInterrupt(timer, &onTimer);
    timerWrite(timer, 0);
    timerAlarm(timer, TIMER_ISR_TICKS, true, 0);
    Serial.printf("Initialisation timer à fe = %d Hz\n", SAMPLE_FREQUENCY);
}


void stopAcquisition()
{
    if (timer != nullptr)
    {
        timerEnd(timer);
        timer = nullptr;
    }
}

uint16_t get_adc_value(uint8_t channel)
{
    return analogRead(ADC_PINS[channel]);

    // Plus tard possible :
    // return adc1_get_raw(...);
}

// ================= ISR =================
void IRAM_ATTR onTimer()
{
    Serial.print(".");
    timerTicks = timerTicks + 1;
    
    // Accumulation oversampling
    for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
    {
        oversampleAcc[ch] += get_adc_value(ch);
    }

    oversampleCount++;

    // Pas encore atteint 2^n
    if (oversampleCount < OVERSAMPLING)
        return;
    
    
    // ===== On produit un échantillon moyen =====

    uint16_t* buffer = getWriteBuffer();
    uint16_t index   = getSampleIndex();
    uint8_t  bufIdx  = getCurrentWriteBufferIndex();

    if (index >= FRAME_SIZE){ // Sécurité
        return;
    }
    Serial.printf(" - %d", index);
    Serial.printf(" - Task core %d\n",xPortGetCoreID());
    
    for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
    {
        // Division rapide par 2^n
        buffer[index + ch] = oversampleAcc[ch] >> OVERSAMPLING_BITS;

        oversampleAcc[ch] = 0;
    }

    oversampleCount = 0;

    index += NB_CAPTEURS;
    setSampleIndex(index);

    if (index >= FRAME_SIZE)
    {
        switchToNextWriteBuffer();
        notifyBufferReadyFromISR(bufIdx);
    }

    /*
    Serial.printf("Task core %d - ",xPortGetCoreID());
    Serial.printf("bufferIdx %d - sampleIdx %d - time %d\n", bufIdx, index, esp_timer_get_time());*/
}
