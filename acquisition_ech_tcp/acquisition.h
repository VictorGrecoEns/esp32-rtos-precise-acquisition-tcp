#ifndef ACQUISITION_H
#define ACQUISITION_H

#include <stdint.h>

void initAcquisition(uint16_t sampleFrequency);
void stopAcquisition();

uint16_t get_adc_value(uint8_t channel);

#endif
