#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
// #include <SPIFFS.h>
// #include <WiFi.h>
#include "config.h"
// #include "wifi_connection/wifi_connection.h"
// #include "wifi_location/wifi_location.h"
// #include "UWB/UWB.h"
// #include "utils/wifi.h"
// #include "Preferences.h"
#include "serial_control/serial_control.h"


std::vector<std::vector<std::string>> scanned_networks; // Stores scanned networks
int number_of_networks_scanned; // Number of networks scanned
Preferences preferences; // Preferences object for storing data


#endif