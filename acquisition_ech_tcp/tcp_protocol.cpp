#include "tcp_protocol.h"
#include "storage.h"
#include "config.h"
#include "wifi_config.h"
#include "acquisition.h"

static TaskHandle_t tcpTaskHandle;
static WiFiClient client;

static volatile uint16_t buffersToSend = 0;       // nombre de buffers restant à envoyer
static volatile bool acquisitionStarted = false; // indique si acquisition active


// Prototype privé
static void tcpTask(void* parameter);
static bool ensureConnected();
static void sendFrame(uint16_t* buffer, uint8_t bufferIndex);

// Initialisation
void initTCP()
{
    Serial.println("Initalisation tache tcp");
    xTaskCreatePinnedToCore(
        tcpTask,        // Function of the task
        "TCP_Task",     // Name of the task
        4096,           // Stack size in words
        NULL,           // Task iniput parameter
        1,              // Priority of the task
        &tcpTaskHandle, // Task handle
        0               // Core where the task should run
    );
}

// Tache TCP
static void tcpTask(void* parameter)
{
    while (true)
    {
        // ===== Connexion TCP =====
        if (!ensureConnected())
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // ===== Vérifie commande serveur =====
       
        if (!acquisitionStarted && client.available() >= 3)
        {
            Serial.printf("Task core %d - ",xPortGetCoreID());
            Serial.println("Vérfication commande serveur");
            uint8_t cmd;
            uint16_t nBuf;
            client.read(&cmd, 1);
            client.read((uint8_t*)&nBuf, 2);  // little-endian

            if (cmd == 1) // START
            {
                buffersToSend = nBuf;   // initialisation du compteur
                acquisitionStarted = true;
                Serial.printf("Task core %d - ",xPortGetCoreID());
                Serial.printf("Acquisition started for %d buffers\n", buffersToSend);
                initAcquisition();     // démarre ISR / timer
            }
        }

        // ===== Envoi des buffers =====
        
        if (acquisitionStarted && buffersToSend > 0)
        {
            
            uint8_t bufferIndex;
            if (waitForReadyBuffer(&bufferIndex))
            {
                Serial.printf("Task core %d - ",xPortGetCoreID());Serial.println("Envoi des buffers");
                uint16_t* buffer = getBufferByIndex(bufferIndex);
                sendFrame(buffer, bufferIndex);

                buffersToSend = buffersToSend - 1;  // décrémente compteur
                if (buffersToSend == 0)
                {
                    stopAcquisition();
                    acquisitionStarted = false;
                    client.println("STOP");  // informe serveur
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


// Connexion TCP
static bool ensureConnected()
{
    if (!WiFiConnected()) {
        Serial.println("Wifi not connected");
        return false;
    }
    if (client.connected()) {
        Serial.println("Client connected");
        return true;
    }
    client.stop();
    Serial.println("Trying to connect to the server...");

    if (client.connect(SERVER_IP, SERVER_PORT)) {
        Serial.println("TCP connection established!");
        return true;
    } else {
        Serial.println("TCP connection failed!");
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
