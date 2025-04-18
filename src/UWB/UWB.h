#ifndef UWB_H
#define UWB_H

#include <SPI.h>
#include "DW1000Ranging.h"
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "config.h"

extern WebServer server;


extern bool isAnchor; // true if the device is an anchor, false if it is a tag
extern float distance; // distance between the tag and the anchor
extern bool isRanging; // true if ranging is active, false otherwise

extern float avgDistance; // average distance calculated from the measurements

void UWB_setup();
void UWB_loop();
void switchMode();

#endif
