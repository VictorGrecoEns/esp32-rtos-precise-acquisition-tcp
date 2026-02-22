#ifndef ACQUISITION_H
#define ACQUISITION_H

#include <stdint.h>

void initAcquisition();
void stopAcquisition();

uint16_t get_adc_value(uint8_t channel);

#endif
