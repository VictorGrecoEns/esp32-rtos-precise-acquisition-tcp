#include "wifi_config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid      = "grecoPi";
const char* password  = "170703Victor.";
const char* serverUrl = "http://192.168.17.7:5000/data";

IPAddress localIP(192,168,17,40);
IPAddress gateway(192,168,17,1);
IPAddress subnet(255,255,255,0);

volatile int nbBlocToSend = -1;

void connect_wifi() {
    WiFi.config(localIP, gateway, subnet);
    WiFi.begin(ssid, password);
    Serial.print("Connexion au wifi..");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
    Serial.println("\nConnecté au wifi !");
}

void get_blocks_from_server() {
    HTTPClient http;
    http.begin("http://192.168.17.7:5000/config");
    if (http.GET() != 200) esp_restart();

    DynamicJsonDocument doc(256);
    deserializeJson(doc, http.getString());
    http.end();

    nbBlocToSend = doc["n_blocks"];
    Serial.print("Réception du nombre de bloc à échantillonner :");
    Serial.println(nbBlocToSend);
}

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
