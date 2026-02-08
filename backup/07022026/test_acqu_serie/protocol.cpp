#include "protocol.h"
#include "wifi_config.h"
#include "timer_config.h"
#include <HTTPClient.h>
/*#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
*/
// Buffers
DataBlock buffers[BUFFER_COUNT];
bool bufferReady[BUFFER_COUNT] = {false};

volatile uint8_t writeBuffer = 0;
volatile uint8_t readBuffer  = 0;

static SemaphoreHandle_t bufferMutex;

// State
volatile bool acquisitionDone = true;

void start_acquisition() {
    acquisitionDone = false;
}

void end_acquisition() {
    acquisitionDone = true;
    nbBlocToSend = -1;
}

void sendDataTask(void* parameter) {
    bufferMutex = xSemaphoreCreateMutex();

    for (;;) {
        bool hasData = false;
        DataBlock localBlock;
        uint8_t blocNumber = 0;

        xSemaphoreTake(bufferMutex, portMAX_DELAY);
        if (bufferReady[readBuffer]) {
            localBlock = buffers[readBuffer];
            blocNumber = readBuffer; 
            bufferReady[readBuffer] = false;
            readBuffer = (readBuffer + 1) % BUFFER_COUNT;
            hasData = true;
        }
        xSemaphoreGive(bufferMutex);

        if (hasData) {
            HTTPClient http;
            http.begin(serverUrl);
            http.addHeader("Content-Type", "application/json");

            String payload;
            payload.reserve(6000);

            payload += "{";
            payload += "\"timestamp\":" + String(localBlock.timestamp) + ",";
            payload += "\"bloc\":" + String(blocNumber) + ","; 
            payload += "\"samples\":[";

            for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
                payload += String(localBlock.samples[i]);
                if (i < BLOCK_SIZE - 1) payload += ",";
            }
            payload += "]}";

            int code = http.POST(payload);
            http.end();

            
            // Serial monitoring
            Serial.print("WiFi Task: bloc envoyé = ");
            Serial.print((int)((readBuffer + BUFFER_COUNT - 1) % BUFFER_COUNT));
            Serial.print(", HTTP code = ");
            Serial.println(code);
        } else {
            vTaskDelay(2);
        }
    }
}
