#include "config.h"

adc_oneshot_unit_handle_t adc1_handle = nullptr;
adc_channel_t adc_channels[NB_CAPTEURS];

static adc_oneshot_chan_cfg_t channel_config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12
};

void initConfig()
{
    Serial.print("Configuration ");

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(&init_config, &adc1_handle);

    for (uint8_t ch = 0; ch < NB_CAPTEURS; ch++)
    {
        pinMode(ADC_PINS[ch], INPUT);

        int8_t ch_id = digitalPinToAnalogChannel(ADC_PINS[ch]);

        if (ch_id < 0) {
            Serial.printf("Erreur pin %d ADC invalide", ADC_PINS[ch]);
            continue;
        }
        
        adc_channels[ch] = static_cast<adc_channel_t>(ch_id);

        adc_oneshot_config_channel(adc1_handle, adc_channels[ch], &channel_config);
    }

    pinMode(PIN_LED, OUTPUT);

    Serial.println("initialized");
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
