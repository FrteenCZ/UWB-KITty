#include "wifi.h"

//scan wifi a save it to scanned_networks vector
void scan_wifi() {
    //clear scanned_networks to ensure no outdated network entries remains
    scanned_networks.clear();

    Serial.println("scanning for networks ...");

    //scan
    number_of_networks_scanned = WiFi.scanNetworks();

    //check if any networks found
    if (number_of_networks_scanned == 0) {
        Serial.println("no networks found");
        return;
    } else {
        //loop through all the networks
        for (int i = 0; i < number_of_networks_scanned; ++i) {
            // Print BSSID, RSSI and SSID for each network found
            Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x",  WiFi.BSSID(i)[0], WiFi.BSSID(i)[1], WiFi.BSSID(i)[2], 
                                                            WiFi.BSSID(i)[3], WiFi.BSSID(i)[4], WiFi.BSSID(i)[5]);
            Serial.print(" - ");
            Serial.print(-WiFi.RSSI(i));
            Serial.print(" - ");
            Serial.print(WiFi.SSID(i));
            Serial.print("\n");

            //saving networks to vector
            //first one
            if (0 == scanned_networks.size()) {
                scanned_networks.push_back({
                        WiFi.SSID(i).c_str(),                   //SSID
                        std::to_string(-WiFi.RSSI(i)),          //RSSI
                        std::to_string(WiFi.encryptionType(i))  //AUTH
                });

            } else {
                //loop through already saved networks
                for (size_t j = 0; j < scanned_networks.size(); j++) {
                    //check if the network is already saved
                    if (std::string(WiFi.SSID(i).c_str()) == scanned_networks[j][0]){
                        break;
                    }
                    
                    //when no saved networks match, save it
                    if (j == scanned_networks.size() - 1) {
                        scanned_networks.push_back({
                                WiFi.SSID(i).c_str(),                   //SSID
                                std::to_string(-WiFi.RSSI(i)),          //RSSI
                                std::to_string(WiFi.encryptionType(i))  //AUTH
                        });
                    }
                }
            }
        }
    }
    Serial.println(number_of_networks_scanned);
}
