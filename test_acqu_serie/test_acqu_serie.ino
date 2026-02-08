#include "timer_config.h"
#include "wifi_config.h"
#include "protocol.h"
#include <esp_timer.h>

constexpr uint8_t PIN_V = 35;
constexpr uint8_t PIN_I = 32;

uint32_t lastTick = 0;
int32_t accV = 0;
int32_t accI = 0;
int8_t  cnt = 0;
uint16_t sampleIndex = 0;
uint64_t acqTimestamp = 0;

void acquisition(void){
    
    while (!acquisitionDone && lastTick < timerTicks) {
        lastTick++; // permet à la boucle de "suivre" le timer matériel

        accV += analogRead(PIN_V); accI += analogRead(PIN_I);
        cnt++;

        if (cnt == 4) {
            cnt = 0;
            uint16_t avgV = accV >> 2; uint16_t avgI = accI >> 2;
            accV = 0; accI = 0;

            if (sampleIndex == 0) {
                nbBlocToSend -= 1;
                if (nbBlocToSend < 0) acquisitionDone = true;
            }

            buffersV[writeBuffer].samples[sampleIndex++] = avgV; buffersI[writeBuffer].samples[sampleIndex++] = avgI;
            
            /* Débug à 100Hz pour etre sur de voir tout dans le bon ordre
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
            Serial.println(esp_timer_get_time());*/
            
            if (sampleIndex >= BLOCK_SIZE) {
                bufferReady[writeBuffer] = true;
                writeBuffer = (writeBuffer + 1) % BUFFER_COUNT;
                acqTimestamp = esp_timer_get_time()
                buffersV[writeBuffer].timestamp = acqTimestamp; buffersI[writeBuffer].timestamp = acqTimestamp;
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
    pinMode(PIN_I, INPUT);
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
    acqTimestamp = esp_timer_get_time()
    buffersV[0].timestamp = acqTimestamp; buffersI[0].timestamp = acqTimestamp;
    timerTicks = 0;
    while(!acquisitionDone){
      acquisition();
    }
    Serial.println("Fin de l'acquisition");
    ledState = true;
    digitalWrite(PIN_LED, LOW);

}


void loop() {
    blink(1, 1000);
}
