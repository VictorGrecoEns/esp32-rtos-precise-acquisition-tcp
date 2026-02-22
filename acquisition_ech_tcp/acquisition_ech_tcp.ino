#include "config.h"
#include "wifi_config.h"
#include "storage.h"
#include "acquisition.h"
#include "tcp_protocol.h"

void initSerial();
void monitorSystem();

void setup()
{
    blinkLED(5, 100);
    initSerial();
    initConfig();
    initStorage();      // crée buffers + queue
    delay(200);
    blinkLED(5, 100);
    initWiFi();
    initTCP();          // crée tâche TCP
    delay(200);

}

void loop()
{
    // monitorSystem();
    // delay(2000);
}

void initSerial()
{
    Serial.begin(115200); delay(200);

    Serial.println("=================================");
    Serial.println("           System boot           ");
    Serial.println("=================================");
}



void monitorSystem()
{
    Serial.printf("Task core %d - ",xPortGetCoreID());
    // Monitoring optionnel (stats, watchdog, etc.)
    
    // Affichage de l'état WiFi
    static uint32_t lastConnectionAttempt = 0;
    static bool wasConnected = false;

    if (WiFiConnected()) {
        // Si connecté et l'état précédent était déconnecté
        if (!wasConnected) {
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            wasConnected = true;
            lastConnectionAttempt = millis();  // Enregistrement du moment de connexion
        }
        
        // Affichage périodique de l'état Wi-Fi
        Serial.print("WiFi Status: Connected | Uptime: ");
        Serial.print((millis() - lastConnectionAttempt) / 1000);  // Affiche le temps depuis la dernière connexion
        Serial.println("s");
        
    } else {
        // Si déconnecté
        if (wasConnected) {
            Serial.println("WiFi disconnected!");
            wasConnected = false;
        }
        
        // Tentative de reconnexion (après un certain délai)
        if (millis() - lastConnectionAttempt > 5000) {  // Vérifie toutes les 5 secondes
            Serial.print("Attempting to reconnect...");
            initWiFi();  // Relance la tentative de connexion
            lastConnectionAttempt = millis();
        }
    }
}
