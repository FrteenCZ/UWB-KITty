#include "wifi_location.h"

WiFiLocation bestMatch;

float calculate_similarity(const JsonObject &networks)
{
    float common = 0;
    int total = networks.size() + number_of_networks_scanned;

    for (size_t scan_index = 0; scan_index < number_of_networks_scanned; scan_index++)
    {
        const char *c_SSID = WiFi.SSID(scan_index).c_str();
        const char *c_BSSID = WiFi.BSSIDstr(scan_index).c_str();
        int c_RSSI = WiFi.RSSI(scan_index);

        Serial.print("\ncomparing: ");
        Serial.print(c_SSID);
        Serial.print(" ");
        Serial.print(c_BSSID);
        Serial.print(" ");
        Serial.println(c_RSSI);

        for (JsonPair kv : networks)
        {
            const char *t_BSSID = kv.key().c_str();
            int t_RSSI = kv.value().as<int>();

            Serial.print("with: ");
            Serial.print(t_BSSID);
            Serial.print(" ");
            Serial.println(t_RSSI);

            float weight = exp((c_RSSI + t_RSSI) / (2*50));


            if (strcmp(c_BSSID, t_BSSID) == 0)
            {
                Serial.println("Common network found");
                Serial.print("Weight: ");
                Serial.println(weight);

                int diff = abs(c_RSSI - t_RSSI);
                Serial.print("RSSI difference: ");
                Serial.println(diff);

                float similarity = 1 - (diff / 100.0);
                Serial.print("Similarity: ");
                Serial.println(similarity);
                
                common += weight * similarity;
                break;
            }
        }
    }
    Serial.print("Common networks: ");
    Serial.println(common);
    Serial.print("Total networks: ");
    Serial.println(total);
    return common / total;
}


void findMatchingLocation(const JsonObject &stored)
{
    bestMatch.name = "X";
    bestMatch.location[0] = 0;
    bestMatch.location[1] = 0;
    bestMatch.location[2] = 0;

    float bestSimilarity = 0;
    float similaritySum = 0;

    Serial.println("Finding matching location");
    for (JsonPair kv : stored)
    {
        const char* locationName = kv.key().c_str();
        JsonObject locationData = kv.value().as<JsonObject>();

        Serial.println("\nProcessing stored object:");
        serializeJson(locationData, Serial);
        Serial.println();

        float similarity = calculate_similarity(locationData["networks"].as<JsonObject>());
        similaritySum += similarity;

        bestMatch.location[0] += locationData["location"][0].as<float>() * similarity;
        bestMatch.location[1] += locationData["location"][1].as<float>() * similarity;
        bestMatch.location[2] += locationData["location"][2].as<float>() * similarity;

        Serial.print("Similarity: ");
        Serial.println(similarity);
        
        if (similarity > bestSimilarity)
        {
            bestSimilarity = similarity;
            bestMatch.name = locationName;
        }
    }
    bestMatch.location[0] /= similaritySum;
    bestMatch.location[1] /= similaritySum;
    bestMatch.location[2] /= similaritySum;
}