#include "wifi_connection.h"

// Helper : Convert Wi-Fi Data to JSON
/**
 * @brief Convert scanned Wi-Fi networks to a JSON string.
 * 
 * This function iterates over the scanned networks and converts them into a JSON array.
 * Each network is represented as a JSON object with SSID, RSSI, and encryption type.
 * 
 * @return String JSON representation of the scanned networks.
 */
String getWiFiJson() {
    JsonDocument doc;  // Create JSON document
    JsonArray arr = doc.to<JsonArray>();

    for (const auto& network : scanned_networks) {
        JsonObject obj = arr.add<JsonObject>();
        obj["ssid"] = network[0];
        obj["rssi"] = network[1];
        obj["encryption"] = network[2];
    }

    String json;
    serializeJson(doc, json);  // Convert JSON object to string
    return json;
}


// Handle root Wi-Fi request
/**
 * @brief Handle HTTP GET request for the root Wi-Fi page.
 * 
 * This function serves the Wi-Fi connection HTML page stored in SPIFFS.
 */
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


// Handle Wi-Fi list request
/**
 * @brief Handle HTTP GET request for the Wi-Fi list.
 * 
 * This function scans for available Wi-Fi networks and sends the list as a JSON response.
 */
void handleWiFiList() {
    scan_wifi();
    server.send(200, "application/json", getWiFiJson());
}


// Handle Wi-Fi connection request
/**
 * @brief Handle HTTP POST request for Wi-Fi connection.
 * 
 * This function parses the SSID and password from the request body and attempts to connect to the Wi-Fi network.
 */
void handleWiFiConnect() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);

    connect_to_wifi(1, 5, true, ssid.c_str(), password.c_str());
}

// Setup Wi-Fi connection routes
/**
 * @brief Setup HTTP routes for Wi-Fi connection.
 * 
 * This function sets up the HTTP routes for handling Wi-Fi connection requests.
 * It registers the handlers for the root Wi-Fi page and the Wi-Fi scan list.
 */
void wifi_connection_setup() {
    server.on("/wifi_connection.html", handleRootWiFi);
    server.on("/wifi_connection", handleRootWiFi);
    server.on("/wifi/scan", handleWiFiList);
    server.on("/wifi/connect", handleWiFiConnect);
}