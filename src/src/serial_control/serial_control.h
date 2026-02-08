#ifndef SERIAL_CONTROL_H
#define SERIAL_CONTROL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "UWB_tracking_logic/trilateration.h"
#include "wifi_connection/wifi_connection.h"
#include "wifi_location/wifi_location.h"
#include "UWB/UWB.h"
#include "../utils/StatusLED.h"

void serialTask();
#endif