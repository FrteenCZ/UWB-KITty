#ifndef LED_TEST_H
#define LED_TEST_H

#include <SPIFFS.h>
#include <WebServer.h>
#include "config.h"

extern WebServer server;
extern String onboardledState;

void LED_test_setup();

void handleRootTest();
void handleLEDOn();
void handleLEDOff();
void handleStatus();
void handleScanNetworks();

#endif