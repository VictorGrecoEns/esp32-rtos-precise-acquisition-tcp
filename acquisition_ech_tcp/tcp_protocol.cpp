#include "tcp_protocol.h"
#include "storage.h"
#include "config.h"
#include "wifi_config.h"
#include "acquisition.h"

static TaskHandle_t tcpTaskHandle;
static WiFiClient client;

static volatile uint16_t buffersToSend = 0;       // number of buffer to send to the server
static volatile bool acquisitionStarted = false;  


// Private/static prototype 
static void tcpTask(void* parameter);
static bool ensureConnected();
static void sendFrame(uint16_t* buffer, uint8_t bufferIndex);

// Initialization
void initTCP()
{
    Serial.print("TCP task ");
    xTaskCreatePinnedToCore(
        tcpTask,        // Function of the task
        "TCP_Task",     // Name of the task
        4096,           // Stack size in words
        NULL,           // Task iniput parameter
        1,              // Priority of the task
        &tcpTaskHandle, // Task handle
        0               // Core where the task should run
    );
    Serial.println("initialized");
}

// Tache TCP
static void tcpTask(void* parameter)
{
    while (true)
    {
        // ===== TCP connexion =====
        if (!ensureConnected())
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // ===== Receiving the acquisition parameters =====
        
        if (!acquisitionStarted && client.available() >= 5)    // acquisition parameters received from the server: [start, n_buffer, freq_ech]
        {   
            // Serial.printf("Task core %d - ",xPortGetCoreID());
            // Serial.println("Vérfication commande serveur");
            uint8_t cmd;
            uint16_t nBuf;
            uint16_t freq;
            client.read(&cmd, 1); // 1 byte
            client.read((uint8_t*)&nBuf, 2);  // 2 bytes in little-endian
            client.read((uint8_t*)&freq, 2);  // 2 bytes in little-endian

            if (cmd == 1) // START
            {
                Serial.print("Acquisition started ");
                // Initialization of the acquisition
                buffersToSend = nBuf;               // count initialized
                acquisitionStarted = true;          // begining the acquisition
                // Serial.printf("Task core %d - ",xPortGetCoreID());

                // applying frequency saturations
                if (freq < 10) {freq = 10;}         //  Low saturation of the acquisition frequency
                if (freq > 10000) {freq = 10000;}   // High saturation of the acquisition frequency

                // Computing the duration of the acquisition
                unsigned long totalSamples = (unsigned long)buffersToSend * N_SAMPLES;
                unsigned long totalSeconds = totalSamples/freq;
                unsigned long restSeconds = totalSamples%freq;

                Serial.printf("for %d buffers at %dHz --> %dh%dmin%ds\n", buffersToSend, freq, totalSeconds / 3600UL, (totalSeconds % 3600UL) / 60UL, totalSeconds % 60UL) ;
                // Begining of the acquisition task
                initAcquisition(freq);     // démarre ISR / timer
            }
        }

        // ===== Sending buffers =====
        
        if (acquisitionStarted && buffersToSend > 0)
        {
            
            uint8_t bufferIndex;
            if (waitForReadyBuffer(&bufferIndex))
            {
                // Serial.printf("Task core %d - ",xPortGetCoreID());
                // Serial.println("Envoi des buffers");
                uint16_t* buffer = getBufferByIndex(bufferIndex);
                sendFrame(buffer, bufferIndex);

                buffersToSend = buffersToSend - 1;  // decreasing the number of buffer to send
                if (buffersToSend == 0){
                    stopAcquisition();
                    acquisitionStarted = false;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // short delay to make sure the core can do others tasks
    }
}


// Connexion TCP
static bool ensureConnected()
{
    if (!WiFiConnected()) {
        // Serial.println("Wifi not connected");
        return false;
    }
    if (client.connected()) {
        // Serial.println("Client connected");
        return true;
    }
    client.stop();
    // Serial.println("Trying to connect to the server...");

    if (client.connect(SERVER_IP, SERVER_PORT)) {
        // Serial.println("TCP connection established!");
        return true;
    } else {
        // Serial.println("TCP connection failed!");
        return false;
    }
}

// Envoi trame 
static void sendFrame(uint16_t* buffer, uint8_t bufferIndex)
{
    if (!client.connected()){
        return;
    }
    uint64_t timestamp = getBufferTimestamp(bufferIndex);
    client.write((uint8_t*)&timestamp, sizeof(timestamp));
    client.write((uint8_t*)buffer, FRAME_BYTES);
}
