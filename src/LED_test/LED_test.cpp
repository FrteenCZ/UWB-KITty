#include "LED_test.h"



void LED_test_setup() {
    pinMode(onboardledPin, OUTPUT);

    server.on("/test", handleRootTest);
    server.on("/led/on", handleLEDOn);
    server.on("/led/off", handleLEDOff);
    server.on("/status", handleStatus);
    server.on("/scan", handleScanNetworks);
}

void handleRootTest() {
    File file = SPIFFS.open("/test.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

void handleLEDOn() {
    onboardledState = "ON";
    digitalWrite(onboardledPin, HIGH);
    server.send(200, "text/plain", "LED is ON");
}

void handleLEDOff() {
    onboardledState = "OFF";
    digitalWrite(onboardledPin, LOW);
    server.send(200, "text/plain", "LED is OFF");
}

void handleStatus() {
    String json = "{ \"led\": \"" + onboardledState + "\" }";
    server.send(200, "application/json", json);
}

void handleScanNetworks() {
    Serial.println("scanning for networks ...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("no networks found");
        return;
    } else {
        for (int i = 0; i < n; ++i) {
            // Print BSSID, RSSI and SSID for each network found
            Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x",  WiFi.BSSID(i)[0], WiFi.BSSID(i)[1], WiFi.BSSID(i)[2], 
                                                            WiFi.BSSID(i)[3], WiFi.BSSID(i)[4], WiFi.BSSID(i)[5]);
            Serial.print(" - ");
            Serial.print(-WiFi.RSSI(i));
            Serial.print(" - ");
            Serial.print(WiFi.SSID(i));
            Serial.print("\n");
        }
    }
}