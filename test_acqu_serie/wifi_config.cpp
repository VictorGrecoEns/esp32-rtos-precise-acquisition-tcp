#include "wifi_config.h"
#include "timer_config.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid      = "grecoPi";
const char* password  = "170703Victor.";
const char* serverUrl = "http://192.168.17.7:5000/data";

IPAddress localIP(192,168,17,40);
IPAddress gateway(192,168,17,1);
IPAddress subnet(255,255,255,0);

void connect_wifi() {
    WiFi.config(localIP, gateway, subnet);
    WiFi.begin(ssid, password);
    Serial.print("Connexion au wifi..");
    
    while (WiFi.status() != WL_CONNECTED) {
        blink(1,250);
        Serial.print(".");
    }
    Serial.println("\nConnecté au wifi !");
}
