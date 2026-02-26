#include "storage.h"
#include "config.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Buffer definition
static uint16_t dataBuffer[NB_BUFFERS][FRAME_SIZE];
static uint64_t bufferTimestamps[NB_BUFFERS] = {0};

// Index definitions
static volatile uint8_t writeBufferIndex = 0;
static volatile uint16_t sampleIndex = 0;

// Queue : 'in' used by the ISR | 'out' used by the TCP Task
static QueueHandle_t bufferReadyQueue = NULL;

// Initialization
void initStorage()
{ 
    Serial.print("Storage ");
    writeBufferIndex = 0;
    sampleIndex = 0;

    bufferReadyQueue = xQueueCreate(NB_BUFFERS, sizeof(uint8_t));
    Serial.println("initialized");
}

// =========== ISR SIDE ===========
uint16_t* getWriteBuffer(){
    return dataBuffer[writeBufferIndex];
}

uint16_t getSampleIndex(){
    return sampleIndex;
}

void setSampleIndex(uint16_t value){
    sampleIndex = value;
}

void incrementSampleIndexBy(uint16_t value){
    sampleIndex = sampleIndex + value;
}

uint8_t getCurrentWriteBufferIndex(){
    return writeBufferIndex;
}

void switchToNextWriteBuffer(){
    writeBufferIndex = (writeBufferIndex + 1) % NB_BUFFERS;
    sampleIndex = 0;
}

void notifyBufferReady(uint8_t bufferIndex){ // implemented if we switch the task to a classic loop ( if no need of ISR )
    bufferTimestamps[bufferIndex] = esp_timer_get_time();
    xQueueSend(bufferReadyQueue, &bufferIndex, portMAX_DELAY);
}

void notifyBufferReadyFromISR(uint8_t bufferIndex){
    bufferTimestamps[bufferIndex] = esp_timer_get_time();
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xQueueSendFromISR(bufferReadyQueue,
                      &bufferIndex,
                      &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint64_t getBufferTimestamp(uint8_t index){
    return bufferTimestamps[index];
}

// =========== TCP SIDE ===========

bool waitForReadyBuffer(uint8_t* bufferIndex){
    if (xQueueReceive(bufferReadyQueue,
                      bufferIndex,
                      portMAX_DELAY) == pdTRUE){
        return true;
    } else {
        return false;
    }
}

uint16_t* getBufferByIndex(uint8_t index){
    return dataBuffer[index];
}
