#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <SPIFFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "utils/wifi.h"

extern WebServer server;
extern std::vector<std::vector<std::string>> scanned_networks;

void handleRootWiFi();
void wifi_connection_setup();
void handleWiFiList();

#endif