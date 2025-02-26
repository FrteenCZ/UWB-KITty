#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

class WiFiLocation
{
    public:
    const char* name;
    float location[3];
};

extern WiFiLocation bestMatch;
extern int number_of_networks_scanned;

void findMatchingLocation(const JsonObject &stored);

#endif // WIFI_LOCATION_H