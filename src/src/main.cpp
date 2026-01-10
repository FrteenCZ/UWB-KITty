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
    
    UWB_setup();
    StatusLED_setup();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    // Handle serial input
    serialTask();

    // UWB processing
    UWB_loop();
}
