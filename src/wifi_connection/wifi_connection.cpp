#include "wifi_connection.h"

// Helper : Convert Wi-Fi Data to JSON
String getWiFiJson() {
    DynamicJsonDocument doc(1024);  // Create JSON document
    JsonArray arr = doc.to<JsonArray>();

    for (const auto& network : scanned_networks) {
        JsonObject obj = arr.createNestedObject();
        obj["ssid"] = network[0];
        obj["rssi"] = network[1];
        obj["encryption"] = network[2];
    }

    String json;
    serializeJson(doc, json);  // Convert JSON object to string
    return json;
}


void wifi_connection_setup() {
    server.on("/wifi", handleRootWiFi);
    server.on("/wifi/scan", handleWiFiList);
}

void handleRootWiFi() {
    File file = SPIFFS.open("/wifi_connection.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

void handleWiFiList() {
    scan_wifi();
    server.send(200, "application/json", getWiFiJson());
}
