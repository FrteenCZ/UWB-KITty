#include "wifi_location.h"

// Global variable to store the best matching location
WiFiLocation bestMatch;

/**
 * @brief Calculate the similarity between scanned networks and stored networks.
 *
 * This function compares the scanned networks with the stored networks and calculates
 * the similarity based on the RSSI values and common networks.
 *
 * @param networks JSON object containing stored networks.
 * @return float Similarity score between 0 and 1.
 */
float calculate_similarity(const JsonObject &networks)
{
    float common = 0;
    int total = networks.size() + number_of_networks_scanned;

    for (size_t scan_index = 0; scan_index < number_of_networks_scanned; scan_index++)
    {
        const char *c_SSID = WiFi.SSID(scan_index).c_str();
        const char *c_BSSID = WiFi.BSSIDstr(scan_index).c_str();
        int c_RSSI = WiFi.RSSI(scan_index);

        Serial.print("\nComparing: ");
        Serial.print(c_SSID);
        Serial.print(" ");
        Serial.print(c_BSSID);
        Serial.print(" ");
        Serial.println(c_RSSI);

        for (JsonPair kv : networks)
        {
            const char *t_BSSID = kv.key().c_str();
            int t_RSSI = kv.value().as<int>();

            Serial.print("With: ");
            Serial.print(t_BSSID);
            Serial.print(" ");
            Serial.println(t_RSSI);

            float weight = exp((c_RSSI + t_RSSI) / (2 * 50));

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

/**
 * @brief Find the best matching location based on scanned networks.
 *
 * This function iterates over the stored locations and calculates the similarity
 * with the scanned networks to find the best matching location.
 *
 * @param stored JSON object containing stored locations and their networks.
 */
void findMatchingLocation(const JsonObject &stored)
{
    bestMatch.name = "Unknown";
    bestMatch.location[0] = 0;
    bestMatch.location[1] = 0;
    bestMatch.location[2] = 0;

    float bestSimilarity = 0;
    float similaritySum = 0;

    Serial.println("Finding matching location");
    for (JsonPair kv : stored)
    {
        const char *locationName = kv.key().c_str();
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

    if (similaritySum > 0)
    {
        bestMatch.location[0] /= similaritySum;
        bestMatch.location[1] /= similaritySum;
        bestMatch.location[2] /= similaritySum;
    }
    else
    {
        Serial.println("No matching location found.");
    }
}

// Handle root location request
/**
 * @brief Handle HTTP GET request for the root location page.
 * 
 * This function serves the location HTML page stored in SPIFFS.
 */
void handleRootLocation()
{
    File file = SPIFFS.open("/location.html", "r");
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

// Handle location request
/**
 * @brief Handle HTTP GET request for the location.
 * 
 * This function scans for available Wi-Fi networks, finds the best matching location,
 * and sends the location as a JSON response.
 */
void handleLocation()
{
    scan_wifi();
    
    // Load stored locations
    File file = SPIFFS.open("/networks.json", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    // Read file into a string
    String content = file.readString();
    file.close();

    // Parse the string into a JSON document
    DynamicJsonDocument doc;
    DeserializationError error = deserializeJson(doc, content);
    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        server.send(500, "text/plain", "Failed to parse JSON");
        return;
    }

    // Extract the JSON object
    JsonObject root = doc.as<JsonObject>();

    // Find the matching location
    findMatchingLocation(root);
    Serial.println(bestMatch.name);
    Serial.printf("Location: %f, %f, %f\n", bestMatch.location[0], bestMatch.location[1], bestMatch.location[2]);

    // Create a JSON object for the response
    DynamicJsonDocument response(1024);
    response["name"] = bestMatch.name;
    response["location"][0] = bestMatch.location[0];
    response["location"][1] = bestMatch.location[1];
    response["location"][2] = bestMatch.location[2];

    // Serialize the JSON object to a string
    String json;
    serializeJson(response, json);

    // Send the JSON response
    server.send(200, "application/json", json);
}

// Setup Wi-Fi location routes
void wifi_location_setup()
{
    server.on("/location", handleLocation);
    server.on("/location.html", handleRootLocation);
}