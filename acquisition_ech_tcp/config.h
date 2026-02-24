#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <WiFi.h>

// LED 
constexpr uint8_t PIN_LED = 2;

// Pins ADC
constexpr uint8_t NB_CAPTEURS = 2;
constexpr uint8_t ADC_PINS[NB_CAPTEURS] = {34, 35}; 

// Paramètres modifiables
constexpr uint16_t N_SAMPLES = 256; // vérifier la taille de la heap pour savoir quelle taille de buffer utile pour quelle fréquence
constexpr uint8_t  NB_BUFFERS = 4;
constexpr uint8_t  OVERSAMPLING_BITS = 2;   // 2 → 2^2 = 4
constexpr uint8_t  OVERSAMPLING = (1 << OVERSAMPLING_BITS);

// Calculs dérivés
constexpr uint32_t FRAME_SIZE  = N_SAMPLES * NB_CAPTEURS;
constexpr uint32_t FRAME_BYTES = FRAME_SIZE * sizeof(uint16_t);


// TCP
extern IPAddress SERVER_IP;
constexpr uint16_t SERVER_PORT = 5000;

void initConfig();
void blinkLED(uint8_t nb, uint8_t t_ms);

#endif
