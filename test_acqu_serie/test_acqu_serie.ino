#include "timer_config.h"
#include "wifi_config.h"
#include "protocol.h"
#include <esp_timer.h>

constexpr uint8_t PIN_V = 35;
uint32_t lastTick = 0;
int32_t acc = 0;
int8_t  cnt = 0;
uint16_t sampleIndex = 0;

void acquisition(void){
    
    while (!acquisitionDone && lastTick < timerTicks) {
        lastTick++; // permet à la boucle de "suivre" le timer matériel

        acc += analogRead(PIN_V);
        cnt++;

        if (cnt == 4) {
            cnt = 0;
            uint16_t avg = acc >> 2;
            acc = 0;

            if (sampleIndex == 0) {
                nbBlocToSend -= 1;
                if (nbBlocToSend < 0) acquisitionDone = true;
            }

            buffers[writeBuffer].samples[sampleIndex++] = avg;
            
            //Débug à 100Hz pour etre sur de voir tout dans le bon ordre
            Serial.print(sampleIndex);
            Serial.print("/");
            Serial.print(BLOCK_SIZE);
            Serial.print(" - ");
            Serial.print(writeBuffer);
            Serial.print(" - ");
            Serial.print(timerTicks);
            Serial.print(" - ");
            Serial.print(lastTick);
            Serial.print(" - ");
            Serial.println(esp_timer_get_time());
            
            if (sampleIndex >= BLOCK_SIZE) {
                bufferReady[writeBuffer] = true;
                writeBuffer = (writeBuffer + 1) % BUFFER_COUNT;
                buffers[writeBuffer].timestamp = esp_timer_get_time();
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
        
    connect_wifi();
    digitalWrite(PIN_LED, HIGH);delay(500);
    blink(2, 100);
    
    get_metadata_from_server();
    
    digitalWrite(PIN_LED, HIGH);delay(500);
    blink(2, 100);
    
    isrBegin();
    
    send_init_to_server(TIMER_ISR_FREQ_HZ / 4, BLOCK_SIZE, nbBlocToSend);
    
    xTaskCreate(sendDataTask, "WiFiTask", 16384, nullptr, 1, nullptr);
    start_acquisition();

    digitalWrite(PIN_LED, HIGH);
    buffers[0].timestamp = esp_timer_get_time();
    timerTicks = 0;
    while(!acquisitionDone){
      acquisition();
    }
    Serial.println("Fin de l'acquisition");
    ledState = true;
    digitalWrite(PIN_LED, LOW);

}


void loop() {
}
