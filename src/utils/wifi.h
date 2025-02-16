#ifndef UTIL_WIFI_H
#define UTIL_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

void scan_wifi();

extern std::vector<std::vector<std::string>> scanned_networks;
extern int number_of_networks_scanned;

#endif