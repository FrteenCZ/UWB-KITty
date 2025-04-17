#ifndef UWB_H
#define UWB_H

#include <SPI.h>
#include "DW1000Ranging.h"
#include <WebServer.h>

#include "config.h"

extern WebServer server;


extern bool isAnchor; // true if the device is an anchor, false if it is a tag


void UWB_setup();
void UWB_loop();

#endif
