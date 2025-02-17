#include "wifi.h"

// Helper: Add network to scanned_networks if it doesn't already exist
void addNetworkIfNotExists(const String &ssid, int rssi, int auth) {
    for (const auto &network : scanned_networks) {
        if (network[0] == std::string(ssid.c_str())) {
            return; // Already exists
        }
    }
    scanned_networks.push_back({
        ssid.c_str(),                   //SSID
        std::to_string(rssi),           //RSSI
        std::to_string(auth)            //AUTH
    });
}

/**
 * @brief Scan available WiFi networks and store unique entries.
 */
void scan_wifi() {
    // Clear previously scanned networks
    scanned_networks.clear();

    Serial.println("scanning for networks ...");

    // Scan for networks
    number_of_networks_scanned = WiFi.scanNetworks();
    if (number_of_networks_scanned == 0) {
        Serial.println("no networks found");
        return;
    }

    // Process each found network
    for (int netIndex = 0; netIndex < number_of_networks_scanned; ++netIndex) {
        // Print BSSID, RSSI and SSID for the current network
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x",  
                    WiFi.BSSID(netIndex)[0], WiFi.BSSID(netIndex)[1], WiFi.BSSID(netIndex)[2], 
                    WiFi.BSSID(netIndex)[3], WiFi.BSSID(netIndex)[4], WiFi.BSSID(netIndex)[5]);
        Serial.print(" - ");
        Serial.print(-WiFi.RSSI(netIndex));
        Serial.print(" - ");
        Serial.print(WiFi.SSID(netIndex));
        Serial.print("\n");

        // Save the network if it's not already in the list
        addNetworkIfNotExists(WiFi.SSID(netIndex), -WiFi.RSSI(netIndex), WiFi.encryptionType(netIndex));
    }
}

/**
 * @brief Attempt to connect to a WiFi network.
 * 
 * If an SSID is provided, attempt to connect directly. Otherwise, scan for available networks,
 * load credentials from SPIFFS (credential.json), and try to connect to any matching network.
 *
 * @param number_of_attempts Number of connection attempts.
 * @param timeout Time (in seconds) to wait for each connection attempt.
 * @param first_scan If true, perform an initial scan before the first attempt.
 * @param SSID SSID to connect to (if provided, use this instead of credentials file).
 * @param Password Password for the provided SSID.
 */
void connect_to_wifi(int number_of_attempts, int timeout, bool first_scan, const String &SSID, const String &Password) {
    // Configure WiFi in station mode and reset connection state
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    for (size_t attempt = 0; attempt < number_of_attempts; attempt++) {
        // Attempt direct connection if an SSID is provided
        if (!SSID.isEmpty()) {
            WiFi.begin(SSID, Password);
            for (size_t sec = 0; sec < timeout; sec++) {
                delay(1000);
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\nSuccessfully connected");
                    return;
                }
                Serial.print(".");
            }   
        } else {
            // For credential-based connection, perform a scan
            if (attempt == 0) {
                if (first_scan) {
                    scan_wifi();
                }
            } else {
                scan_wifi();
            }

            // Open and parse the credentials file from SPIFFS
            File file = SPIFFS.open("/credential.json", FILE_READ); 
            if (!file) {
                Serial.println("Failed to open file for reading");
                return;
            }

            JsonDocument doc;                                        
            DeserializationError error = deserializeJson(doc, file);
            file.close();

            if (error) {
                Serial.println("Failed to parse JSON");
                return;
            }

            // Access the JSON arrays for SSIDs and Passwords
            JsonArray SSIDs = doc["SSIDs"];
            JsonArray Passwords = doc["Passwords"];


            // Loop through stored credentials
            for (size_t credIndex = 0; credIndex < SSIDs.size(); credIndex++) {
                String autoSSID = SSIDs[credIndex].as<const char*>();           
                String autoPassword = Passwords[credIndex].as<const char*>();   


                // Check if the scanned networks contain the credential's SSID
                for (const auto &network : scanned_networks) {
                    if (network[0] == std::string(autoSSID.c_str())) {
                        Serial.printf("Connecting to \"%s\"\n", autoSSID.c_str());
                        WiFi.begin(autoSSID, autoPassword);
                        for (int sec = 0; sec < timeout; sec++) {
                            delay(1000);
                            if (WiFi.status() == WL_CONNECTED) {
                                Serial.println("\nSuccessfully connected");
                                return;
                            }
                            Serial.print(".");
                        }
                        Serial.println("");
                    }
                }
            }
        }
    }
    Serial.println("unable to connect to any network");
}