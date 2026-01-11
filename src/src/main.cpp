#include "main.h"
#include <unordered_map>
#include <string>
#include "UWB_tracking_logic/matrix.h"
#include "UWB_tracking_logic/trilateration.h"
#include <WebServer.h>
#include "utils/StatusLED.h"
#include "utils/Button.h"

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
    
    StatusLED_setup();
    UWB_setup();
    Button_setup();
    Serial.println("Setup complete.");


    StatusLED_setColor(50, 0, 50);
}

void loop()
{
    // Handle serial input
    serialTask();

    // Update button state
    Button_update();

    // UWB processing
    UWB_loop();
}
