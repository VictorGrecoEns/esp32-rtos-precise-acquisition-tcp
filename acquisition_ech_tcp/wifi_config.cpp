#include "wifi_config.h"
#include <WiFi.h>
#include <stdint.h>

static const char* WIFI_SSID = "grecoPi";
static const char* WIFI_PASSWORD = "170703Victor.";

void initWiFi()
{
    Serial.print("Connecting to WiFi\n...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t startAttemptTime = millis();

    // Timeout 10 secondes
    while (!WiFiConnected() &&
           millis() - startAttemptTime < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFiConnected())
    {
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("WiFi connection FAILED");
        esp_restart();
    }
}

bool WiFiConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}
