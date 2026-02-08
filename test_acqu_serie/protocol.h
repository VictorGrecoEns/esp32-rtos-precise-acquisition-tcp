#pragma once
#include <Arduino.h>

constexpr uint8_t  BUFFER_COUNT = 8;
constexpr uint16_t BLOCK_SIZE   = 512;

extern volatile int nbBlocToSend;
extern volatile uint32_t freqEch;

struct DataBlock {
    uint64_t timestamp;
    uint16_t samples[BLOCK_SIZE];
};

void start_acquisition();
void end_acquisition();

void get_metadata_from_server();
void send_init_to_server(uint32_t fe, uint16_t block_size, uint32_t n_blocks);
void sendDataTask(void* parameter);

// Buffers circulaires
extern DataBlock buffers[BUFFER_COUNT];
extern bool      bufferReady[BUFFER_COUNT];

extern volatile uint8_t writeBuffer;
extern volatile uint8_t readBuffer;

extern volatile bool acquisitionDone;
