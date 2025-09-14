#include "main.h"

String onboardledState = "OFF";


void setup()
{
    Serial.begin(115200);

    onboardledState = "on";
    digitalWrite(onboardledPin, HIGH);
}

void loop()
{
    // Handle serial input
    handleSerialInput();
}
