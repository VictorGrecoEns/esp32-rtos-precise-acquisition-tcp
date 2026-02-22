#include "config.h"
IPAddress SERVER_IP(192, 168, 17, 7);
void initConfig()
{
    Serial.println("Initialisation configuration");
    for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
    {
        pinMode(ADC_PINS[ch], INPUT);
    }
    pinMode(PIN_LED, OUTPUT);
    // Vérification cohérence paramètres si nécessaire
}

void blinkLED(uint8_t nb, uint8_t t_ms)
{
    bool ledState = false;
    digitalWrite(PIN_LED, ledState);
    for (int i = 0; i < 2*nb; i++){
        digitalWrite(PIN_LED, ledState);
        ledState = !ledState;
        delay(t_ms);
    }
    digitalWrite(PIN_LED, false);
}
