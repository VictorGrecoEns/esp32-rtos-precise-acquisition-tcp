#pragma once
#include <Arduino.h>

void connect_wifi();
void get_blocks_from_server();
void send_init_to_server(uint32_t fe, uint16_t block_size, uint32_t n_blocks);

extern const char* serverUrl;
extern volatile int nbBlocToSend;
