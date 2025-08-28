#include "main.h"

String onboardledState = "OFF";

// Create WiFi server on port 80
WebServer server(80);

void handleRoot()
{
    File file = SPIFFS.open("/index.html", "r");
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

void handleCss()
{
    File file = SPIFFS.open("/style.css", "r");
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String css = file.readString();
    file.close();
    server.send(200, "text/css", css);
}

void handleNotFound()
{
    Serial.print("Unhandled request: ");
    Serial.println(server.uri()); // Print the requested URL
    server.send(404, "text/plain", "Not Found");
}

void setup()
{
    Serial.begin(115200);

    // Set onboard LED pin as output
    pinMode(onboardledPin, OUTPUT);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    { // true -> format if failed
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted Successfully");

    // connect_to_wifi(1, 5, true, "SSID", "PASSWORD");
    if (WiFi.status() != WL_CONNECTED)
    {
        connect_to_wifi();
        if (WiFi.status() != WL_CONNECTED)
        {
            start_AP("ESP32-AP", "12345678");
        }
    }

    LED_test_setup();
    wifi_connection_setup();
    wifi_location_setup();
    UWB_setup();

    // Setup routes
    server.on("/", handleRoot);
    server.on("/style.css", handleCss);

    server.onNotFound(handleNotFound);

    server.begin();

    onboardledState = "on";
    digitalWrite(onboardledPin, HIGH);
}

void loop()
{
    // Handle serial input
    handleSerialInput();

    server.handleClient();
    UWB_loop();
}
