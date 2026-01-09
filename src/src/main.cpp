#include "main.h"
#include <unordered_map>
#include <string>
#include "UWB_tracking_logic/matrix.h"
#include "UWB_tracking_logic/trilateration.h"
#include <WebServer.h>
#include "utils/StatusLED.h"

// Define the global server instance
WebServer server(80);

//            __..--''``---....___   _..._    __
//  /// //_.-'    .-/";  `        ``<._  ``.''_ `. / // /
// ///_.-' _..--.'_    \                    `( ) ) // //
// / (_..-' // (< _     ;_..__               ; `' / ///
//  / // // //  `-._,_)' // / ``--...____..-' /// / //

String onboardledState = "OFF";


void setup()
{
    Serial.begin(115200);

    // pinMode(onboardledPin, OUTPUT);

    onboardledState = "on";
    // digitalWrite(onboardledPin, HIGH);

    pinMode(BUTTON_PIN, INPUT_PULLUP);


    
}

void loop()
{
    // Handle serial input
    handleSerialInput();

  if (digitalRead(BUTTON_PIN) == LOW) {
    // Button pressed
    StatusLED_setColor(0, 255, 0); // Green
  } else {
    // Button released
    StatusLED_setColor(0, 0, 0); // Off
  }
    
}
