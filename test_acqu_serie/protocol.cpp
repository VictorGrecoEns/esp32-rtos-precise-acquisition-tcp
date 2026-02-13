#include "protocol.h"
#include "wifi_config.h"
#include "timer_config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

/*#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
*/
// ============================================================
//                   Définition des buffers
// ============================================================
DataBlock buffers[BUFFER_COUNT];
bool bufferReady[BUFFER_COUNT];

volatile uint8_t writeBuffer = 0;
volatile uint8_t readBuffer  = 0;

static SemaphoreHandle_t bufferMutex;



// ============================================================
//                  Début et fin d'acquisition
// ============================================================
// State
volatile bool acquisitionDone = true;

void start_acquisition() {
    for (uint8_t i = 0; i<BUFFER_COUNT; i++){
      bufferReady[BUFFER_COUNT] = false;
    }
    acquisitionDone = false;
}

void end_acquisition() {
    acquisitionDone = true;
    nbBlocToSend = -1;
}


// ============================================================
//               Envoi d'initialisation au serveur
// ============================================================
void send_init_to_server(uint32_t fe, uint16_t block_size, uint32_t n_blocks) {
    HTTPClient http;
    http.begin("http://192.168.17.7:5000/init");
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"fe\":" + String(fe) + ",";
    payload += "\"block_size\":" + String(block_size) + ",";
    payload += "\"n_blocks\":" + String(n_blocks);
    payload += "}";

    if (http.POST(payload) != 200) esp_restart();
    http.end();
}


// ============================================================
//           Réception des métadonnées
// ============================================================
volatile int nbBlocToSend = -1;
volatile uint32_t freqEch = 1;

void get_metadata_from_server() {
    uint8_t counter = 250;
    Serial.print("Waiting the server's metadatas...");
    HTTPClient http;
    http.begin("http://192.168.17.7:5000/config");
    int httpGet = http.GET();
    while ( httpGet != 200 && counter>=0) {
      blink(2,100);
      Serial.print(".");
      counter -= 1;
      httpGet = http.GET();
    }
    if (httpGet != 200 || counter < 0){
      esp_restart();
    }

    DynamicJsonDocument doc(512);
    deserializeJson(doc, http.getString());
    http.end();

    nbBlocToSend = doc["n_blocks"];
    freqEch      = doc["fe"];
    Serial.print("\nNumber of blocks to aquire:");
    Serial.println(nbBlocToSend);
    Serial.print("Frequency to acquire at: ");
    Serial.print(freqEch);
    Serial.println("Hz");
}


// ============================================================
//               Envoi des blocs au serveur
// ============================================================
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
            payload.reserve(6000); // timestamp: 34, bloc: 8, samplesX: 13+4N+(N-1), {,,,}:5 --> 10*N+71

            payload += "{";
            payload += "\"timestamp\":" + String(localBlock.timestamp) + ",";
            payload += "\"bloc\":" + String(blocNumber) + ","; 
            payload += "\"samples1\":[";

            for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
                payload += String(localBlock.samples1[i]);
                if (i < BLOCK_SIZE - 1) payload += ",";
            }
            payload += "],\"samples2\":[";

            for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
                payload += String(localBlock.samples2[i]);
                if (i < BLOCK_SIZE - 1) payload += ",";
            }
            payload += "]}";

            int code = http.POST(payload);
            http.end();

            
            // Serial monitoring
            Serial.print(localBlock.timestamp);
            Serial.print(" - WiFi Task: sent block = ");
            Serial.print((int)((BUFFER_COUNT + readBuffer - 1) % BUFFER_COUNT));
            Serial.print(", HTTP code = ");
            Serial.println(code);
        } else {
            vTaskDelay(2);
        }
    }
}
