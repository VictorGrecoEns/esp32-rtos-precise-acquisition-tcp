#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <WiFi.h>
#include "esp_adc/adc_oneshot.h"

// <--------- : editable parameters

// LED pin
constexpr uint8_t PIN_LED = 2;

// ADC pins
constexpr uint8_t NB_CAPTEURS = 2;                    // <---------
constexpr uint8_t ADC_PINS[NB_CAPTEURS] = {34, 35};   // <---------
extern adc_oneshot_unit_handle_t adc1_handle;
extern adc_channel_t adc_channels[NB_CAPTEURS];


// Buffer parameters
constexpr uint16_t N_SAMPLES = 256;                   // <---------
constexpr uint8_t  NB_BUFFERS = 4; // vérifier la taille de la heap pour savoir quelle taille de buffer utile pour quelle fréquence

// Oversampling parameters
constexpr uint8_t  OVERSAMPLING_BITS = 2;   // 2 → 2^2 = 4
constexpr uint8_t  OVERSAMPLING = (1 << OVERSAMPLING_BITS);

// Computed parameters
constexpr uint32_t FRAME_SIZE  = N_SAMPLES * NB_CAPTEURS;
constexpr uint32_t FRAME_BYTES = FRAME_SIZE * sizeof(uint16_t);


// Server parameters
constexpr const char* SERVER_IP = "192.168.17.7"; 
constexpr uint16_t  SERVER_PORT = 5000;                // <---------

// Prototypes
void initConfig();
void blinkLED(uint8_t nb, uint8_t t_ms);

#endif
