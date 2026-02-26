#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

// ===== Initialization =====
void initStorage();

// ===== ISR SIDE =====
uint16_t* getWriteBuffer();
uint16_t  getSampleIndex();
void      setSampleIndex(uint16_t value);
void      incrementSampleIndexBy(uint16_t value);

uint8_t   getCurrentWriteBufferIndex();
void      switchToNextWriteBuffer();

void      notifyBufferReadyFromISR(uint8_t bufferIndex);
void      notifyBufferReady(uint8_t bufferIndex);
uint64_t  getBufferTimestamp(uint8_t index);

// ===== TASK SIDE =====
bool      waitForReadyBuffer(uint8_t* bufferIndex);
uint16_t* getBufferByIndex(uint8_t index);

#endif
