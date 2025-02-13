#include <ArduinoJson.h>
#include <Arduino.h>
#include <vector>
#include "SPIFFS.h"
#include "WiFi.h"
#include <variant>

#define CONTROL_LED_MODE 0
#define WIFI_CONNECT_MODE 1

int mode = 0;
std::vector<std::vector<std::string>> scanned_networks; //vector containg all networks around

void auto_connect_to_wifi();
void manual_connect_to_wifi();


// Create WiFi server on port 80
WiFiServer server(80);




void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {  // true -> format if failed
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted Successfully");
    manual_connect_to_wifi();
    //auto_connect_to_wifi();
}

void loop() {
    switch (mode)
    {
    case CONTROL_LED_MODE:
        break;
    
    case WIFI_CONNECT_MODE:
        
        break;
    default:
        break;
    }
}

//connecting to saved connections
void auto_connect_to_wifi() {
    //scan networks
        //setup wifi
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        //scan
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


    //open credential.json
        //open the file using SPIFFS
        File file = SPIFFS.open("/credential.json", FILE_READ); 

        //check if the file does not exist or cannot be opened
        if (!file) {
            Serial.println("Failed to open file for reading");
            return;
        }

        //read file
        JsonDocument doc;                                        //create a Json document
        DeserializationError error = deserializeJson(doc, file); //reads the file and parses the JSON into the doc object
        file.close();

        //check if file is valid json
        if (error) {
            Serial.println("Failed to parse JSON");
            return;
        }

        // Access the JSON arrays 
        JsonArray SSIDs = doc["SSIDs"];
        JsonArray PASSWORDs = doc["PASSWORDs"];


    //check if any saved networks match the scanned ones    
        for (size_t j = 0; j < SSIDs.size(); j++) {
        
            //file network credetials
            String SSID = SSIDs[j].as<const char*>();     //SSID
            String PASS = PASSWORDs[j].as<const char*>(); //password

            Serial.printf("Network \"%s\" ", SSID);

            //iterate through scanned networks
            for (int i = 0; i < n; ++i) {

                //check if they match
                if (WiFi.SSID(i) == SSID) {
                    Serial.println("found");
                    Serial.printf("Connecting to \"%s\"\n", SSID);

                //connect to that network
                WiFi.begin(SSID, PASS);
                while (WiFi.status() != WL_CONNECTED) {
                    delay(1000);
                    Serial.print(".");
                }
                Serial.print("\nsuccessfully connected\n");

                return;
                }
            }
            Serial.println("not found");
        }
        Serial.println("unable to connect to any network");
        return;
}

//manually connecting the ESP32 to wifi using access point
void manual_connect_to_wifi() {
    //setup wifi
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-SoftAP", "123456789");
    Serial.println("SoftAP Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    //start server
    server.begin();

    //scan
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
            Serial.print(" - ");
            Serial.print(WiFi.encryptionType(i));
            Serial.print("\n");
            
            if (0 == scanned_networks.size()) {
                scanned_networks.push_back({
                        WiFi.SSID(i).c_str(),
                        std::to_string(-WiFi.RSSI(i)),
                        std::to_string(WiFi.encryptionType(i))
                });
            } else {
                for (size_t j = 0; j < scanned_networks.size(); j++) {
                    if (std::string(WiFi.SSID(i).c_str()) == scanned_networks[j][0]){
                        if (-WiFi.RSSI(i) < std::stoi(scanned_networks[j][1])) {
                            scanned_networks[j][1] = std::to_string(-WiFi.RSSI(i));
                        }

                        break;
                    }
                    
                    if (j == scanned_networks.size() - 1) {
                        scanned_networks.push_back({
                                WiFi.SSID(i).c_str(),
                                std::to_string(-WiFi.RSSI(i)),
                                std::to_string(WiFi.encryptionType(i))
                        });
                    }
                }
            }
        }
    }
    Serial.println("sex");
    for (const auto& network : scanned_networks) {
        Serial.printf("SSID: %s, RSSI: %s, Encryption: %s\n", 
                      network[0].c_str(), network[1].c_str(), network[2].c_str());
    }

    mode = 1;
}
