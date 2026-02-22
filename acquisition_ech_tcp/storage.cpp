#include "storage.h"
#include "config.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Buffer linéaire contigu
static uint16_t dataBuffer[NB_BUFFERS][FRAME_SIZE];
static uint64_t bufferTimestamps[NB_BUFFERS] = {0};

// Index gestion
static volatile uint8_t writeBufferIndex = 0;
static volatile uint16_t sampleIndex = 0;

// Queue ISR → TCP Task
static QueueHandle_t bufferReadyQueue = NULL;

// Initialisation
void initStorage()
{ 
    Serial.println("Initialisation storage");
    writeBufferIndex = 0;
    sampleIndex = 0;

    bufferReadyQueue = xQueueCreate(NB_BUFFERS, sizeof(uint8_t));
    
}

// ISR SIDE
uint16_t* getWriteBuffer()
{
    return dataBuffer[writeBufferIndex];
}

uint16_t getSampleIndex()
{
    return sampleIndex;
}

void setSampleIndex(uint16_t value)
{
    sampleIndex = value;
}

void incrementSampleIndexBy(uint16_t value)
{
    sampleIndex = sampleIndex + value;
}

uint8_t getCurrentWriteBufferIndex()
{
    return writeBufferIndex;
}

void switchToNextWriteBuffer()
{
    writeBufferIndex = (writeBufferIndex + 1) % NB_BUFFERS;
    sampleIndex = 0;
}

void notifyBufferReadyFromISR(uint8_t bufferIndex)
{
    bufferTimestamps[bufferIndex] = esp_timer_get_time();
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(bufferReadyQueue,
                      &bufferIndex,
                      &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint64_t getBufferTimestamp(uint8_t index)
{
    return bufferTimestamps[index];
}

// TCP SIDE

bool waitForReadyBuffer(uint8_t* bufferIndex)
{
    if (xQueueReceive(bufferReadyQueue,
                      bufferIndex,
                      portMAX_DELAY) == pdTRUE)
    {
        return true;
    }

    return false;
}

uint16_t* getBufferByIndex(uint8_t index)
{
    return dataBuffer[index];
}
