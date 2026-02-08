#include "timer_config.h"
#include "wifi_config.h"
#include "protocol.h"
#include <esp_timer.h>

constexpr uint8_t PIN_V = 35;

void acquisition(void){
    static uint32_t lastTick = 0;
    static uint32_t acc = 0;
    static uint8_t  cnt = 0;
    static uint16_t sampleIndex = 0;
    static uint32_t lastSerial = 0;
    
    while (!acquisitionDone && lastTick < timerTicks) {
        lastTick++;

        acc += analogRead(PIN_V);
        cnt++;

        if (cnt == 4) {
            cnt = 0;
            uint16_t avg = acc >> 2;
            acc = 0;

            if (sampleIndex == 0) {
                buffers[writeBuffer].timestamp = esp_timer_get_time();
                nbBlocToSend -= 1;
                if (nbBlocToSend < 0) acquisitionDone = true;
            }

            buffers[writeBuffer].samples[sampleIndex++] = avg;

            if (sampleIndex >= BLOCK_SIZE) {
                bufferReady[writeBuffer] = true;
                writeBuffer = (writeBuffer + 1) % BUFFER_COUNT;
                sampleIndex = 0;
            }
        }
    }
}


void setup() {
    Serial.begin(115200);
    delay(100);
    
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_V, INPUT);
    digitalWrite(PIN_LED, LOW);
    
    isrBegin();
    
    connect_wifi();
    get_blocks_from_server();
    
    send_init_to_server(TIMER_ISR_FREQ_HZ / 4, BLOCK_SIZE, nbBlocToSend);

    xTaskCreate(sendDataTask, "WiFiTask", 16384, nullptr, 1, nullptr);
    start_acquisition();
    while(!acquisitionDone){
      acquisition();
    }
    Serial.println("Fin de l'acquisition");
}


void loop() {
    
}
