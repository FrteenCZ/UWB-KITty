#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "utils/wifi.h"

class WiFiLocation
{
    public:
    const char* name;
    float location[3];
};

extern WiFiLocation bestMatch;
extern int number_of_networks_scanned;
extern WebServer server;

void findMatchingLocation(const JsonObject &stored);
void wifi_location_setup();

#endif // WIFI_LOCATION_H