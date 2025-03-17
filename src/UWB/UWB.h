#ifndef UWB_H
#define UWB_H

//#include <DW1000.h>
#include <DW1000Ranging.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "esp_task_wdt.h"


#include "config.h"

extern WebServer server;

void UWB_setup();
void UWB_loop();

#endif
