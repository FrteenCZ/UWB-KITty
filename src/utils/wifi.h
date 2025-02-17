#ifndef UTIL_WIFI_H
#define UTIL_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <ArduinoJson.h>
#include <SPIFFS.h>

void scan_wifi();
void connect_to_wifi(int number_of_attempts = 1, int timeout = 5, bool first_scan = true, const String &SSID = "", const String &Password = "");


extern std::vector<std::vector<std::string>> scanned_networks;
extern int number_of_networks_scanned;

#endif